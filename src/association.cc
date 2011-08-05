#include "association.h"

namespace superfastmatch
{
  bool result_sorter(Result const& lhs,Result const& rhs ){
    if (lhs.length!=rhs.length)
      return lhs.length > rhs.length;
    if (lhs.text.compare(rhs.text)!=0)
      return lhs.text < rhs.text;
    if (lhs.left!=rhs.left)
      return lhs.left < rhs.left;
    return lhs.right < rhs.right;
  }
  
  Association::Association(Registry* registry,Document* from_document,Document* to_document):
  registry_(registry),
  from_document_(from_document),
  to_document_(to_document),
  results_(new vector<Result>())
  {
    key_=new string(from_document->getKey());
    key_->append(to_document->getKey());
    reverse_key_=new string(to_document->getKey());
    reverse_key_->append(from_document->getKey());
    if(not load()){
      match();
    }
  }
  
  Association::~Association(){
    delete key_;
    delete reverse_key_;
    delete results_;
  }
  
  string& Association::getKey(){
    return *key_;
  }
  
  string& Association::getReverseKey(){
    return *reverse_key_;
  }
  
  bool Association::load(){
    string value;
    bool flip=false;
    if(not registry_->getAssociationDB()->get(getKey(),&value)){
      return false; 
    }
    if (value=="x"){
      if (not registry_->getAssociationDB()->get(getReverseKey(),&value)){
        return false;
      }
      flip=true;
    }
    size_t offset=0;
    uint64_t left,right,length;
    while(offset<value.size()){
      offset+=kc::readvarnum(value.data()+offset,5,flip?&right:&left);
      offset+=kc::readvarnum(value.data()+offset,5,flip?&left:&right);
      offset+=kc::readvarnum(value.data()+offset,5,&length);
      results_->push_back(Result(left,right,from_document_->text().substr(left,length),length));
    }
    return true;
  }
  
  bool Association::save(){
    bool success=true;
    char* value=new char[getResultCount()*5*3];
    size_t offset=0;
    for (size_t i=0;i<results_->size();i++){
      offset+=kc::writevarnum(value+offset,results_->at(i).left);
      offset+=kc::writevarnum(value+offset,results_->at(i).right);
      offset+=kc::writevarnum(value+offset,results_->at(i).length);
    }
    if (not registry_->getAssociationDB()->set(getKey().data(),16,value,offset)){
      success=false;
    }
    if (not registry_->getAssociationDB()->set(getReverseKey().data(),16,"x",1)){
      success=false;
    }
    delete value;
    return success;
  }
  
  void Association::match(){
    // TODO Optimise for shorter document
    // if (from_document->text().length()<to_document->text().length()){
    Document::hashes_bloom* bloom = new Document::hashes_bloom();
    Document::hashes_vector from_hashes,to_hashes;
    *bloom|=from_document_->bloom();
    *bloom&=to_document_->bloom();
    from_hashes=from_document_->hashes();
    to_hashes=to_document_->hashes();
    uint32_t from_hashes_count = from_hashes.size();
    uint32_t to_hashes_count = to_hashes.size();
    string from_text = from_document_->getCleanText();
    string to_text = to_document_->getCleanText();
    uint32_t window_size=registry_->getWindowSize();
    hash_t white_space=registry_->getWhiteSpaceHash(false);

    //Find from_document hashes set
    hashes_set from_hashes_set;
    for (size_t i=0;i<from_hashes_count;i++){
      if ((bloom->test(from_hashes[i]&0xFFFFFF))&&(from_hashes[i]!=white_space)){
        from_hashes_set.insert(from_hashes[i]);
      }
    }

    //Find to_document hashes map
    matches_map to_matches;
    hashes_set::iterator from_hashes_set_end=from_hashes_set.end();
    hash_t hash;
    for (size_t i=0;i<to_hashes_count;i++){
      hash=to_hashes[i];
      if ((bloom->test(hash&0xFFFFFF))&&(hash!=white_space)){
        if (from_hashes_set.find(hash)!=from_hashes_set_end){
          to_matches[hash].insert(i);   
        }
      }
    }

    //Build matches
    matches_deque matches;
    matches_map::iterator to_matches_end=to_matches.end();
    for (size_t i=0;i<from_hashes_count;i++){
      matches_map::iterator to_match=to_matches.find(from_hashes[i]);
      if (to_match!=to_matches_end){
        positions_set checked_matches(to_match->second);
        for (positions_set::iterator it=checked_matches.begin();it!=checked_matches.end();++it){
          if (from_text.compare(i,window_size,to_text,*it,window_size)){
            checked_matches.erase(it);
          }
        }
        matches.push_back(Match(i,checked_matches));
      }  
    }

    //Process matches to find longest common strings
    while(matches.size()>0){
       Match* first = &matches.front();
       if (first->right.size()==0){
          matches.pop_front();
          continue; //Skip loop and progress forwards!
       }
       uint32_t first_right=*first->right.begin();
       first->right.erase(first->right.begin());
       uint32_t counter=1;
       while(counter<matches.size()){
          Match* next = &matches[counter];;
          if ((next->left-first->left)==counter){
             positions_set::iterator next_right=next->right.find(first_right+counter);
             if (next_right!=next->right.end()){
                next->right.erase(*next_right);
                counter++;
             }
             else{
                break;
             }
          }
          else{
             break;
          }
       }
       results_->push_back(Result(first->left,first_right,from_text.substr(first->left,counter+window_size),counter+window_size));
    }
    sort(results_->begin(),results_->end(),result_sorter);
    delete bloom;
  }

  size_t Association::getTotalLength(){
    size_t total=0;
    for (size_t i=0;i<results_->size();i++){
      total+=getLength(i);
    }
    return total;
  }
  
  size_t Association::getResultCount(){
    return results_->size();
  }
  
  string Association::getFromResult(size_t index){
    return from_document_->text().substr(results_->at(index).left,results_->at(index).length);
  }
  
  string Association::getToResult(size_t index){
    return to_document_->text().substr(results_->at(index).right,results_->at(index).length);
  }
  
  size_t Association::getLength(size_t index){
    return results_->at(index).length;
  }
  
  void Association::fill_item_dictionary(TemplateDictionary* dict){
    uint32_t previous_left=numeric_limits<uint32_t>::max();
    uint32_t previous_right=numeric_limits<uint32_t>::max();
    uint32_t previous_length=0;
    string previous_text="";
    string text;
    TemplateDictionary* fragment_dict;
    TemplateDictionary* left_dict;
    TemplateDictionary* right_dict;
    for (size_t i=0;i<results_->size();i++){
      text=from_document_->text().substr(results_->at(i).left,results_->at(i).length);
      if (text.compare(previous_text)!=0){
        fragment_dict=dict->AddSectionDictionary("FRAGMENT");
        fragment_dict->SetValue("TITLE",to_document_->title());
        fragment_dict->SetIntValue("DOC_TYPE",to_document_->doctype());
        fragment_dict->SetIntValue("DOC_ID",to_document_->docid());
        fragment_dict->SetIntValue("LENGTH",results_->at(i).length);
        fragment_dict->SetValue("TEXT",text);
      }
      if (previous_left!=results_->at(i).left){
        left_dict=fragment_dict->AddSectionDictionary("LEFT_POSITIONS");
        left_dict->SetIntValue("LEFT_POSITION",results_->at(i).left);
      }
      if(previous_right!=results_->at(i).right){
        right_dict=fragment_dict->AddSectionDictionary("RIGHT_POSITIONS");
        right_dict->SetIntValue("RIGHT_POSITION",results_->at(i).right);
      }
      previous_text=text;
      previous_left=results_->at(i).left;
      previous_right=results_->at(i).right;
      previous_length=results_->at(i).length;
    }
  }
  
  void Association::fill_list_dictionary(TemplateDictionary* dict){
    
  }
}
