#include "association.h"

namespace superfastmatch
{
  bool result_sorter(Result const& lhs,Result const& rhs ){
     if (lhs.length!=rhs.length)
        return lhs.length > rhs.length;
     if (lhs.left!=rhs.left)
        return lhs.left < rhs.left;
     return lhs.right < rhs.right;
  }
  
  Association::Association(Registry* registry,Document* from_document,Document* to_document):
  registry_(registry),
  from_document_(0),
  to_document_(0)
  {
    if (from_document->text().length()<to_document->text().length()){
      from_document_=from_document;
      to_document_=to_document;
    }
    else{
      from_document_=to_document;
      to_document_=from_document;
    }
    match();
    save();
  }
  
  void Association::match(){
    Document::hashes_bloom* bloom = new Document::hashes_bloom();
    Document::hashes_vector from_hashes,to_hashes;
    *bloom|=from_document_->bloom();
    *bloom&=to_document_->bloom();
    from_hashes=from_document_->hashes();
    to_hashes=to_document_->hashes();
    uint32_t from_hashes_count = from_hashes.size();
    uint32_t to_hashes_count   = to_hashes.size();
    std::string from_text = from_document_->text();
    std::string to_text = to_document_->text();
    uint32_t window_size=registry_->getWindowSize();

    //Find from_document hashes set
    hashes_set from_hashes_set;
    for (uint32_t i =0;i<from_hashes_count;i++){
      if (bloom->test(from_hashes[i]&0xFFFFFF)){
        from_hashes_set.insert(from_hashes[i]);
      }
    }

    //Find to_document hashes map
    matches_map to_matches;
    // to_matches.set_empty_key(NULL);
    hashes_set::iterator from_hashes_set_end=from_hashes_set.end();
    hash_t hash;
    for (uint32_t i=0;i<to_hashes_count;i++){
      hash=to_hashes[i];
      if (bloom->test(hash&0xFFFFFF)){
      // if (bloom.test(hash&0xFFFFFF)){
        if (from_hashes_set.find(hash)!=from_hashes_set_end){
          to_matches[hash].insert(i);   
        }
      }
    }

    //Build matches
    matches_deque matches;
    matches_map::iterator to_matches_end=to_matches.end();
    for (uint32_t i=0;i<from_hashes_count;i++){
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
       results_.push_back(Result(first->left,first_right,counter+window_size));
    }
    sort(results_.begin(),results_.end(),result_sorter);

    std::cout << "Matched: " << *from_document_ << " with: " << *to_document_;
    std::cout << " Bloom size: " << bloom->count()<< " From hashes: " << from_document_->hashes().size() <<" To hashes: " << to_document_->hashes().size();
    std::cout << " From hashes set: " << from_hashes_set.size() << " To hashes map: " << to_matches.size() << " Matches: " << matches.size();
    std::cout << " Results: " << results_.size() << std::endl;

    delete bloom;
  }
  
  bool Association::load(){
    return true;
  }
  
  bool Association::save(){
    return true;    
  }
  
  void Association::fill_item_dictionary(TemplateDictionary* dict){
    TemplateDictionary* association_dict=dict->AddIncludeDictionary("ASSOCIATION");
    association_dict->SetFilename(ASSOCIATION);
    association_dict->SetValue("FROM_TITLE",from_document_->title());
    association_dict->SetIntValue("FROM_DOC_TYPE",from_document_->doctype());
    association_dict->SetIntValue("FROM_DOC_ID",from_document_->docid());
    association_dict->SetValue("TO_TITLE",to_document_->title());
    association_dict->SetIntValue("TO_DOC_TYPE",to_document_->doctype());
    association_dict->SetIntValue("TO_DOC_ID",to_document_->docid());
    for (size_t i=0;i<results_.size();i++){
      TemplateDictionary* fragment_dict=association_dict->AddSectionDictionary("FRAGMENT");
      fragment_dict->SetIntValue("LEFT_POSITION",results_[i].left);
      fragment_dict->SetIntValue("RIGHT_POSITION",results_[i].right);
      fragment_dict->SetIntValue("LENGTH",results_[i].length);
      fragment_dict->SetValue("LEFT",from_document_->text().substr(results_[i].left,results_[i].length));
      fragment_dict->SetValue("RIGHT",to_document_->text().substr(results_[i].right,results_[i].length));
    }
  }
  
  void Association::fill_list_dictionary(TemplateDictionary* dict){
    
  }
}
