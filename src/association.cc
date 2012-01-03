#include "association.h"

namespace superfastmatch
{
  //-----------------------
  // Instrument definitions
  //-----------------------
  
  enum AssociationTimers{
    MATCH,
    INIT_MATCH
  };

  enum AssociationCounters{
    FROM_DOC_TYPE,
    FROM_DOC_ID,
    TO_DOC_TYPE,
    TO_DOC_ID,
    WINDOW_SIZE,
    THRESHOLD,
    FROM_HASHES,
    TO_HASHES,
    FROM_BLOOM,
    TO_BLOOM,
    BLOOM,
    TOO_SHORT,
    ABOVE_THRESHOLD,
    TOTAL_CHARACTERS,
    RESULTS,
  };
    
  template<> const InstrumentDefinition Instrumented<Association>::getDefinition(){
    return InstrumentDefinition("Association",MATCH,create_map<int32_t,string>(MATCH,"Match")(INIT_MATCH,"Init Match"),
                                create_map<int32_t,string>(FROM_DOC_TYPE,"From Doc Type")
                                                          (FROM_DOC_ID,"From Doc Id")
                                                          (TO_DOC_TYPE,"To Doc Type")
                                                          (TO_DOC_ID,"To Doc Id")
                                                          (WINDOW_SIZE,"Window Size")
                                                          (THRESHOLD,"Threshold")
                                                          (FROM_HASHES,"From Hashes")
                                                          (TO_HASHES,"To Hashes")
                                                          (FROM_BLOOM,"From Bloom")
                                                          (TO_BLOOM,"To Bloom")
                                                          (BLOOM,"Bloom")
                                                          (TOO_SHORT,"Too Short")
                                                          (ABOVE_THRESHOLD,"Above T'hold")
                                                          (TOTAL_CHARACTERS,"Total Chars")
                                                          (RESULTS,"Results"));
  }
  
  bool result_sorter_left(Result const& lhs,Result const& rhs){
    return lhs.left< rhs.left;
  }
  
  bool result_sorter_right(Result const& lhs,Result const& rhs){
    return lhs.right< rhs.right;
  }
  
  Association::Association(Registry* registry,DocumentPtr from_document,DocumentPtr to_document):
  registry_(registry),
  from_document_(from_document),
  to_document_(to_document),
  results_(new vector<Result>())
  {
    getInstrument()->setCounter(FROM_DOC_TYPE,from_document->doctype());
    getInstrument()->setCounter(FROM_DOC_ID,from_document->docid());
    getInstrument()->setCounter(TO_DOC_TYPE,from_document->doctype());
    getInstrument()->setCounter(TO_DOC_ID,from_document->docid());
    key_=new string(from_document->getKey());
    key_->append(to_document->getKey());
    reverse_key_=new string(to_document->getKey());
    reverse_key_->append(from_document->getKey());
    if(not load()){
      // TODO The state should be indicated by the document itself to permit lazy evalution like this
      if ((to_document_->doctype()!=0)&&(to_document_->docid()!=0)){
        to_document_=registry_->getDocumentManager()->getDocument(to_document_->doctype(),to_document_->docid(),DocumentManager::TEXT|DocumentManager::HASHES|DocumentManager::BLOOM|DocumentManager::META); 
      }
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
    uint64_t left,right,length,uc_left,uc_right,uc_length;
    while(offset<value.size()){
      offset+=kc::readvarnum(value.data()+offset,5,flip?&right:&left);
      offset+=kc::readvarnum(value.data()+offset,5,flip?&left:&right);
      offset+=kc::readvarnum(value.data()+offset,5,&length);
      offset+=kc::readvarnum(value.data()+offset,5,flip?&uc_right:&uc_left);
      offset+=kc::readvarnum(value.data()+offset,5,flip?&uc_left:&uc_right);
      offset+=kc::readvarnum(value.data()+offset,5,&uc_length);
      results_->push_back(Result(left,right,length,uc_left,uc_right,uc_length,from_document_->getText().substr(left,length)));
    }
    return true;
  }
  
  bool Association::save(){
    assert((from_document_->doctype()!=0)&&(from_document_->docid()!=0));
    assert((to_document_->doctype()!=0)&&(to_document_->docid()!=0));
    bool success=true;
    // 6 variable length 32-bit unsigned integers (maximum 5 bytes each)
    size_t value_size=getResultCount()*5*6;
    char* value=new char[value_size];
    size_t offset=0;
    for (size_t i=0;i<results_->size();i++){
      offset+=kc::writevarnum(value+offset,results_->at(i).left);
      offset+=kc::writevarnum(value+offset,results_->at(i).right);
      offset+=kc::writevarnum(value+offset,results_->at(i).length);
      offset+=kc::writevarnum(value+offset,results_->at(i).uc_left);
      offset+=kc::writevarnum(value+offset,results_->at(i).uc_right);
      offset+=kc::writevarnum(value+offset,results_->at(i).uc_length);
    }
    if (not registry_->getAssociationDB()->set(getKey().data(),16,value,offset)){
      success=false;
    }
    if (not registry_->getAssociationDB()->set(getReverseKey().data(),16,"x",1)){
      success=false;
    }
    delete[] value;
    return success;
  }
  
  bool Association::remove(){
    return (registry_->getAssociationDB()->remove(getKey())&&
            registry_->getAssociationDB()->remove(getReverseKey()));
  }
  
  void Association::match(){
    getInstrument()->startTimer(MATCH);
    getInstrument()->startTimer(INIT_MATCH);
    const bool invert=to_document_->getText().length()<from_document_->getText().length();
    if (invert){
     from_document_.swap(to_document_); 
    }
    hashes_vector from_hashes,to_hashes;
    hashes_bloom* bloom = new hashes_bloom();
    *bloom|=from_document_->getBloom();
    *bloom&=to_document_->getBloom();
    from_hashes=from_document_->getHashes();
    to_hashes=to_document_->getHashes();
    unordered_set<string> above_threshold;
    const uint32_t from_hashes_count = from_hashes.size();
    const uint32_t to_hashes_count = to_hashes.size();
    string original_text = from_document_->getText();
    const uint32_t window_size=registry_->getWindowSize();
    const uint32_t white_space=registry_->getWhiteSpaceHash(false);
    const uint32_t threshold=registry_->getMaxPostingThreshold();
    getInstrument()->setCounter(WINDOW_SIZE,window_size);
    getInstrument()->setCounter(THRESHOLD,threshold);
    getInstrument()->setCounter(FROM_HASHES,from_hashes_count);
    getInstrument()->setCounter(TO_HASHES,to_hashes_count);
    // getInstrument()->setCounter(FROM_BLOOM,from_document_->getBloom().count());
    // getInstrument()->setCounter(TO_BLOOM,to_document_->getBloom().count());
    // getInstrument()->setCounter(BLOOM,bloom->count());
    getInstrument()->stopTimer(INIT_MATCH);

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
            getInstrument()->incrementCounter(TOO_SHORT);
            // logger->log(Logger::DEBUG,kc::strprintf("Bad Match: \"%s\" : \"%s\"",from_document_->getCleanText(i,window_size).c_str(),to_document_->getCleanText(*it,window_size).c_str()).c_str());
          }
        }
        if (match.right.size()>0){
          matches.push_back(match);
        }
      }else if(to_match!=to_matches_end && to_match->second.size()>threshold){
        above_threshold.insert(from_document_->getCleanText(i,window_size));
      }
    }
    getInstrument()->setCounter(ABOVE_THRESHOLD,above_threshold.size());
    
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
         Result result(left,right,length,0,0,0,original_text.substr(left,length));
         results_->push_back(result);
         getInstrument()->incrementCounter(TOTAL_CHARACTERS,length);
         getInstrument()->incrementCounter(RESULTS);
         // logger->log(Logger::DEBUG,kc::strprintf("Match: \"%s\"",result.text.c_str()).c_str()); 
       }else{
         getInstrument()->incrementCounter(TOO_SHORT);
         // logger->log(Logger::DEBUG,kc::strprintf("Match too short: \"%s\"",text.c_str()).c_str());
       }
    }
    
    // Fix up unicode sections
    sort(results_->begin(),results_->end(),result_sorter_right);
    size_t to_cursor=0;
    size_t to_uc_cursor=0;
    const char* to_text=to_document_->getText().data();
    for(vector<Result>::iterator it=results_->begin(),ite=results_->end();it!=ite;++it){
      while((it->right)>to_cursor){
        u8_inc(to_text,&to_cursor);
        to_uc_cursor++;
      }
      it->uc_right=to_uc_cursor;
      size_t length_cursor=to_cursor;
      while((it->length+to_cursor)>length_cursor){
        u8_inc(to_text,&length_cursor);
        it->uc_length++;
      }
    }
    
    sort(results_->begin(),results_->end(),result_sorter_left);
    size_t from_cursor=0;
    size_t from_uc_cursor=0;
    const char* from_text=from_document_->getText().data();
    for(vector<Result>::iterator it=results_->begin(),ite=results_->end();it!=ite;++it){
      while((it->left)>from_cursor){
        u8_inc(from_text,&from_cursor);
        from_uc_cursor++;
      }
      it->uc_left=from_uc_cursor;
    }
    
    // Check for flip
    if (invert){
     from_document_.swap(to_document_);
     for (vector<Result>::iterator it=results_->begin(),ite=results_->end();it!=ite;++it){
       swap(it->left,it->right);
       swap(it->uc_left,it->uc_right);
     }
    }
    delete bloom;
    getInstrument()->stopTimer(MATCH);
  }

  size_t Association::getTotalLength(){
    size_t total=0;
    for (size_t i=0;i<results_->size();i++){
      total+=results_->at(i).length;
    }
    return total;
  }
  
  size_t Association::getResultCount(){
    return results_->size();
  }

  const Result& Association::getResult(const size_t index){
    return results_->at(index);
  }
  
  const string Association::getFromResultText(size_t index){
    return from_document_->getText().substr(results_->at(index).left,results_->at(index).length);
  }
  
  const string Association::getToResultText(size_t index){
    return to_document_->getText().substr(results_->at(index).right,results_->at(index).length);
  }
  
  void Association::fillJSONDictionary(TemplateDictionary* dict,set<string>& metadata){
    TemplateDictionary* docDict=dict->AddSectionDictionary("DOCUMENT");
    vector<string> keys;
    if (to_document_->getMetaKeys(keys)){
      for (vector<string>::iterator it=keys.begin();it!=keys.end();it++){
        TemplateDictionary* metaDict=docDict->AddSectionDictionary("META");
        metaDict->SetValue("KEY",*it);
        metaDict->SetValue("VALUE",to_document_->getMeta(&(*it->c_str())));
        metadata.insert(*it);
      }
    }
    for(vector<Result>::const_iterator it=results_->begin(),ite=results_->end();it!=ite;++it){
      TemplateDictionary* fragmentDict=docDict->AddSectionDictionary("FRAGMENT");
      fragmentDict->SetIntValue("FROM",it->uc_left);
      fragmentDict->SetIntValue("TO",it->uc_right);
      fragmentDict->SetIntValue("LENGTH",it->uc_length);
      fragmentDict->SetIntValue("HASH",it->hash);
    }
  }
  
  // -------------------
  // Association Manager
  // -------------------
  
  AssociationManager::AssociationManager(Registry* registry):
  registry_(registry){}
  
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
}
