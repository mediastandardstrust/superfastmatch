#include "association.h"

namespace superfastmatch
{
  RegisterTemplateFilename(ASSOCIATION, "association.tpl");
  
  bool result_sorter(Result const& lhs,Result const& rhs ){
    if (lhs.length!=rhs.length)
      return lhs.length > rhs.length;
    if (lhs.text.compare(rhs.text)!=0)
      return lhs.text < rhs.text;
    if (lhs.left!=rhs.left)
      return lhs.left < rhs.left;
    return lhs.right < rhs.right;
  }
  
  Association::Association(Registry* registry,DocumentPtr from_document,DocumentPtr to_document):
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
      results_->push_back(Result(left,right,from_document_->getText().substr(left,length),length));
    }
    return true;
  }
  
  bool Association::save(){
    assert((from_document_->doctype()!=0)&&(from_document_->docid()!=0));
    assert((to_document_->doctype()!=0)&&(to_document_->docid()!=0));
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
  
  bool Association::remove(){
    return (registry_->getAssociationDB()->remove(getKey())&&
            registry_->getAssociationDB()->remove(getReverseKey()));
  }
  
  void Association::match(){
    // TODO Optimise for shorter document
    // if (from_document->getText().length()<to_document->getText().length())
    Logger* logger = registry_->getLogger();
    stringstream message;
    hashes_bloom* bloom = new hashes_bloom();
    hashes_vector from_hashes,to_hashes;
    *bloom|=from_document_->getBloom();
    *bloom&=to_document_->getBloom();
    from_hashes=from_document_->getHashes();
    to_hashes=to_document_->getHashes();
    uint32_t from_hashes_count = from_hashes.size();
    uint32_t to_hashes_count = to_hashes.size();
    string original_text = from_document_->getText();
    uint32_t window_size=registry_->getWindowSize();
    uint32_t white_space=registry_->getWhiteSpaceHash(false);
    uint32_t threshold=registry_->getMaxPostingThreshold();
    size_t bad_matches=0;
    size_t above_threshold=0;

    // cout << "From length: " << from_document_->getText().size() << " To length: " << to_document_->getText().size() << " Bloom: " << bloom->count() << " From: " << from_document_->getBloom().count() << " To: " << to_document_->getBloom().count() << endl;

    //Find from_document hashes set
    uint32_t from_hash;
    hashes_set from_hashes_set;
    // from_hashes_set.rehash(from_hashes_count);
    for (size_t i=0;i<from_hashes_count;i++){
      from_hash=from_hashes[i];
      if ((bloom->test(from_hash&0x3FFFFFF))&&(from_hash!=white_space)){ 
        from_hashes_set.insert(from_hash);
      }
    }
    // cout << from_hashes_set.bucket_count() << ":" << from_hashes_set.load_factor() << ":" << from_hashes_set.max_load_factor() << ":" << from_hashes_set.size() << ":" << from_hashes_count << endl;
    //Find to_document hashes map
    uint32_t to_hash;
    matches_map to_matches;
    // to_matches.rehash(to_hashes_count);
    hashes_set::iterator from_hashes_set_end=from_hashes_set.end();
    for (size_t i=0;i<to_hashes_count;i++){
      to_hash=to_hashes[i];
      if ((bloom->test(to_hash&0x3FFFFFF))&&(to_hash!=white_space)){ 
        if (from_hashes_set.find(to_hash)!=from_hashes_set_end){
          to_matches[to_hash].insert(i);
        }
      }
    }
    // cout << to_matches.bucket_count() << ":" << to_matches.load_factor() << ":" << to_matches.max_load_factor() << ":" << to_matches.size() << ":" << to_hashes_count << endl;

    //Build matches
    matches_deque matches;
    matches_map::iterator to_matches_end=to_matches.end();
    for (size_t i=0;i<from_hashes_count;i++){
      matches_map::iterator to_match=to_matches.find(from_hashes[i]);
      if (to_match!=to_matches_end && to_match->second.size()<=threshold){
        Match match(i);
        string from_text = from_document_->getCleanText(i,window_size);
        for (positions_set::iterator it=to_match->second.begin(),ite=to_match->second.end();it!=ite;++it){
          if (from_text==to_document_->getCleanText(*it,window_size)){
            match.right.insert(*it);
          }else{
            // logger->log(Logger::DEBUG,kc::strprintf("Bad Match: \"%s\" : \"%s\"",from_document_->getCleanText(i,window_size).c_str(),to_document_->getCleanText(*it,window_size).c_str()).c_str());
            bad_matches++;
          }
        }
        if (match.right.size()>0){
          matches.push_back(match);
        }
      }else if(to_match!=to_matches_end && to_match->second.size()>threshold){
        above_threshold++;
        // TODO save these cases in a separate way
        // cout << to_match->second.size() << ":" << from_document_->getCleanText(i,window_size) << endl; 
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
       
       // Trim leading and following whitespace
       uint32_t length=counter-1+window_size;
       string text=from_document_->getCleanText(first->left,length);
       text.erase(text.begin(), std::find_if(text.begin(),text.end(),std::not1(std::ptr_fun<int, int>(std::isspace))));
       uint32_t left=first->left+length-text.size();
       uint32_t right=first_right+length-text.size();
       text.erase(std::find_if(text.rbegin(),text.rend(),std::not1(std::ptr_fun<int, int>(std::isspace))).base(), text.end());
       length=text.size();

       // Keep if text is as long as window size
       if (length>=window_size){
         Result result(left,right,original_text.substr(left,length),length);
         results_->push_back(result);
         // logger->log(Logger::DEBUG,kc::strprintf("Match: \"%s\"",result.text.c_str()).c_str()); 
       }else{
         // logger->log(Logger::DEBUG,kc::strprintf("Match too short: \"%s\"",text.c_str()).c_str());
       }
    }
    sort(results_->begin(),results_->end(),result_sorter);
    message << "Associated: " << *from_document_ << " with: " << *to_document_ << " Matches: " << results_->size() << " Bad Matches: " << bad_matches << " Above Threshold: " << above_threshold;
    logger->log(Logger::DEBUG,&message);
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
    return from_document_->getText().substr(results_->at(index).left,results_->at(index).length);
  }
  
  string Association::getToResult(size_t index){
    return to_document_->getText().substr(results_->at(index).right,results_->at(index).length);
  }
  
  size_t Association::getLength(size_t index){
    return results_->at(index).length;
  }
  
  void Association::fillItemDictionary(TemplateDictionary* dict){
    uint32_t previous_left=numeric_limits<uint32_t>::max();
    uint32_t previous_right=numeric_limits<uint32_t>::max();
    uint32_t previous_length=0;
    string previous_text="";
    string text;
    TemplateDictionary* fragment_dict;
    TemplateDictionary* left_dict;
    TemplateDictionary* right_dict;
    for (size_t i=0;i<results_->size();i++){
      text=from_document_->getCleanText(results_->at(i).left,results_->at(i).length);
      if (text.compare(previous_text)!=0){
        fragment_dict=dict->AddSectionDictionary("FRAGMENT");
        fragment_dict->SetValue("TITLE",to_document_->getMeta("title"));
        fragment_dict->SetIntValue("DOC_TYPE",to_document_->doctype());
        fragment_dict->SetIntValue("DOC_ID",to_document_->docid());
        fragment_dict->SetIntValue("LENGTH",results_->at(i).length);
        fragment_dict->SetValue("TEXT",from_document_->getText().substr(results_->at(i).left,results_->at(i).length));
      }
      if (previous_left!=results_->at(i).left || previous_length!=results_->at(i).length){
        left_dict=fragment_dict->AddSectionDictionary("LEFT_POSITIONS");
        left_dict->SetIntValue("LEFT_POSITION",results_->at(i).left);
      }
      if(previous_right!=results_->at(i).right || previous_length!=results_->at(i).length){
        right_dict=fragment_dict->AddSectionDictionary("RIGHT_POSITIONS");
        right_dict->SetIntValue("RIGHT_POSITION",results_->at(i).right);
      }
      previous_text=text;
      previous_left=results_->at(i).left;
      previous_right=results_->at(i).right;
      previous_length=results_->at(i).length;
    }
  }
  
  // Association Manager
  // -------------------
  
  AssociationManager::AssociationManager(Registry* registry):
  registry_(registry){}
  
  AssociationManager::~AssociationManager(){}

  vector<AssociationPtr> AssociationManager::createTemporaryAssociations(DocumentPtr doc){
    assert((doc->doctype()==0)&&(doc->docid()==0));
    return createAssociations(doc,false);
  }
  
  vector<AssociationPtr> AssociationManager::createPermanentAssociations(DocumentPtr doc){
    assert((doc->doctype()!=0)&&(doc->docid()!=0));
    return createAssociations(doc,true);
  }
  
  vector<AssociationPtr> AssociationManager::createAssociations(DocumentPtr doc,const bool save){
    vector<AssociationPtr> associations;
    search_t results;
    inverted_search_t pruned_results;
    registry_->getPostings()->searchIndex(doc,results,pruned_results);
    size_t num_results=registry_->getNumResults();
    size_t count=0;
    for(inverted_search_t::iterator it2=pruned_results.begin(),ite2=pruned_results.end();it2!=ite2 && count<num_results;++it2){
      DocumentPtr other=registry_->getDocumentManager()->getDocument(it2->second.doc_type,it2->second.doc_id);
      AssociationPtr association(new Association(registry_,doc,other));
      associations.push_back(association);
      if (save){
        association->save();
      }
      count++;
    }
    return associations;
  }
  
  bool AssociationManager::removeAssociations(DocumentPtr doc){
    bool success=true;
    vector<AssociationPtr> associations=getAssociations(doc,DocumentManager::NONE);
    for (vector<AssociationPtr>::iterator it=associations.begin(),ite=associations.end();it!=ite;++it){
      success&=(*it)->remove();
    }
    return success;
  }
  
  vector<AssociationPtr> AssociationManager::getAssociations(DocumentPtr doc, const int32_t state){
    vector<AssociationPtr> associations;
    vector<string> keys;
    registry_->getAssociationDB()->match_prefix(doc->getKey().substr(0,8),&keys);
    for(vector<string>::const_iterator it=keys.begin(),ite=keys.end();it!=ite;++it){
      DocumentPtr other = registry_->getDocumentManager()->getDocument((*it).substr(8,8),state);
      associations.push_back(AssociationPtr(new Association(registry_,doc,other)));
    }
    return associations;
  }
  
  void AssociationManager::fillListDictionary(DocumentPtr doc,TemplateDictionary* dict){
    TemplateDictionary* association_dict=dict->AddIncludeDictionary("ASSOCIATION");
    association_dict->SetFilename(ASSOCIATION);
    vector<AssociationPtr> associations=getAssociations(doc,DocumentManager::META);
    for (vector<AssociationPtr>::iterator it=associations.begin(),ite=associations.end();it!=ite;++it){
      (*it)->fillItemDictionary(association_dict);
    } 
  }
  
  void AssociationManager::fillSearchDictionary(DocumentPtr doc,TemplateDictionary* dict){
    TemplateDictionary* association_dict=dict->AddIncludeDictionary("ASSOCIATION");
    association_dict->SetFilename(ASSOCIATION);
    vector<AssociationPtr> associations=createTemporaryAssociations(doc);
    for (vector<AssociationPtr>::iterator it=associations.begin(),ite=associations.end();it!=ite;++it){
      (*it)->fillItemDictionary(association_dict);
    }
  }
}
