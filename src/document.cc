#include "document.h"

namespace superfastmatch
{
  RegisterTemplateFilename(DOCUMENTS_JSON, "JSON/documents.tpl");
  
  //-----------------------
  // Instrument definitions
  //-----------------------
  
  enum DocumentTimers{
    INIT,
    META,
    TEXT,
    HASHES,
    POSTING_HASHES,
    BLOOM
  };

  enum DocumentCounters{
    DOC_TYPE,
    DOC_ID,
    WINDOW_SIZE,
    HASH_WIDTH
  };
    
  template<> const InstrumentDefinition Instrumented<Document>::getDefinition(){
    return InstrumentDefinition("Document",INIT,create_map<int32_t,string>(INIT,"Init")
                                                                          (META,"Meta")
                                                                          (TEXT,"Text")
                                                                          (HASHES,"Hashes")
                                                                          (POSTING_HASHES,"Posting Hashes")
                                                                          (BLOOM,"Bloom"),
                                                create_map<int32_t,string>(DOC_TYPE,"Doc Type")
                                                                          (DOC_ID,"Doc Id")
                                                                          (WINDOW_SIZE,"Window Size")
                                                                          (HASH_WIDTH,"Hash Width"));
  }
  
  // -----------------------
  // Document members
  // -----------------------

  Document::Document(const uint32_t doctype,const uint32_t docid, const bool permanent,Registry* registry):
  doctype_(doctype),docid_(docid),permanent_(permanent),empty_meta_(new string()),registry_(registry),key_(0),text_(0),metadata_(),hashes_(0),posting_hashes_(0),bloom_(0)
  {
    getInstrument()->setCounter(DOC_TYPE,doctype);
    getInstrument()->setCounter(DOC_ID,docid);
    getInstrument()->setCounter(WINDOW_SIZE,registry->getWindowSize());
    getInstrument()->setCounter(HASH_WIDTH,registry->getHashWidth());
    char key[8];
    uint32_t dt=kc::hton32(doctype_);
    uint32_t di=kc::hton32(docid_);
    memcpy(key,&dt,4);
    memcpy(key+4,&di,4);
    key_ = new string(key,8);
  }
  
  Document::Document(const string& key,const bool permanent,Registry* registry):
  permanent_(permanent),empty_meta_(new string()),registry_(registry),key_(0),text_(0),metadata_(0),hashes_(0),posting_hashes_(0),bloom_(0)
  {
    key_= new string(key);
    memcpy(&doctype_,key.data(),4);
    memcpy(&docid_,key.data()+4,4);
    doctype_=kc::ntoh32(doctype_);
    docid_=kc::ntoh32(docid_);
  }
  
  Document::~Document(){
    if (metadata_!=0){
      delete metadata_;
      metadata_=0;
    }
    if (hashes_!=0){
      delete hashes_;
      hashes_=0;
    }
    if (posting_hashes_!=0){
      delete posting_hashes_;
      posting_hashes_=0;
    }
    if (bloom_!=0){
      delete bloom_;
      bloom_=0; 
    }
    if(key_!=0){
      delete key_;
      key_=0;
    }
    if (text_!=0){
      delete text_;
      text_=0;
    }
    if (empty_meta_!=0){
      delete empty_meta_;
      empty_meta_=0;
    }
  }
  
  bool Document::remove(){
    vector<string> keys;
    vector<string> meta_keys;
    vector<string> ordered_keys;
    assert(getMetaKeys(keys));
    for (vector<string>::iterator it=keys.begin(),ite=keys.end();it!=ite;++it){
      string meta_key=getKey()+*it;
      string ordered_key=*it+padIfNumber(getMeta(*it))+getKey();
      meta_keys.push_back(meta_key);
      ordered_keys.push_back(ordered_key);
    }
    return registry_->getDocumentDB()->remove(*key_) &&\
           (registry_->getMetaDB()->remove_bulk(meta_keys)!=-1) &&\
           (registry_->getOrderedMetaDB()->remove_bulk(ordered_keys)!=-1);
  }

  bool Document::setMeta(const string& key, const string& value){
    if (metadata_==0){
      throw runtime_error("Meta not initialised");
    }
    (*metadata_)[key]=value;
    if (permanent_){
      string meta_key=getKey()+key;
      string old_ordered_key=key+padIfNumber(getMeta(key))+getKey();
      string new_ordered_key=key+padIfNumber(value)+getKey();
      registry_->getOrderedMetaDB()->remove(old_ordered_key);
      return registry_->getMetaDB()->set(meta_key,value) &&\
             registry_->getOrderedMetaDB()->set(new_ordered_key,"");
    }
    return true;
  }
  
  bool Document::initMeta(){
    if (metadata_==0){
      getInstrument()->startTimer(META);
      metadata_map* tempMap = new metadata_map();
      vector<string> meta_keys;
      string meta_value;
      registry_->getMetaDB()->match_prefix(getKey(),&meta_keys);
      for (vector<string>::const_iterator it=meta_keys.begin(),ite=meta_keys.end();it!=ite;++it){
        if (not registry_->getMetaDB()->get(*it,&meta_value)){
          getInstrument()->stopTimer(META);
          return false;
        };
        (*tempMap)[(*it).substr(8)]=meta_value;
      }      
      metadata_=tempMap;
    }
    getInstrument()->stopTimer(META);
    return true;
  }
  
  string& Document::getMeta(const string& key){
    if (metadata_==0){
      throw runtime_error("Meta not initialised");
    }
    if (metadata_->find(key)!=metadata_->end()){
      return (*metadata_)[key];
    }
    return *empty_meta_;
  }
  
  bool Document::getMetaKeys(vector<string>& keys){
    keys.clear();
    if (metadata_!=0){
      for (metadata_map::const_iterator it=metadata_->begin(),ite=metadata_->end();it!=ite;it++){
        keys.push_back(it->first);
      }
      return true;
    }
    return false;
  }

  bool Document::initText(){
    bool success=true;
    if (text_==0){
      getInstrument()->startTimer(TEXT);
      text_ = new string();
      success=registry_->getDocumentDB()->get(getKey(),text_);
      getInstrument()->stopTimer(TEXT);
    }
    return success;
  }

  bool Document::initHashes(){
    if (hashes_==0){
      getInstrument()->startTimer(HASHES);
      hashes_vector* tempHashes = new hashes_vector();
      if(getText().size()>=registry_->getWindowSize()){
        UpperCaseRabinKarp(getText(),registry_->getWindowSize(),registry_->getWhiteSpaceThreshold(),*tempHashes);
      }
      hashes_=tempHashes;
      getInstrument()->stopTimer(HASHES);
    }
    return true;
  }

  bool Document::initPostingHashes(){
    if (posting_hashes_==0){
      getInstrument()->startTimer(POSTING_HASHES);
      hashes_vector* tempHashes = new hashes_vector();
      if(getText().size()>=registry_->getWindowSize()){
        UpperCaseRabinKarp(getText(),registry_->getPostingWindowSize(),registry_->getWhiteSpaceThreshold(),*tempHashes);
      }
      posting_hashes_=tempHashes;
      getInstrument()->stopTimer(POSTING_HASHES);
    }
    return true;
  }

  bool Document::initBloom(){
    if (bloom_==0){
      getInstrument()->startTimer(BLOOM);
      if (hashes_==0){
        throw runtime_error("Hashes not initialised, needed for Bloom");
      }
      hashes_bloom* tempBloom = new hashes_bloom();
      for (hashes_vector::const_iterator it=getHashes().begin(),ite=getHashes().end();it!=ite;++it){
        tempBloom->set((*it)&0x3FFFFFF);
      }
      bloom_=tempBloom;
      getInstrument()->stopTimer(BLOOM);
    }
    return true;
  }
  
  bool Document::isPermanent(){
    return permanent_;
  }
  
  hashes_vector& Document::getHashes(){
    if (hashes_==0){
      throw runtime_error("Hashes not initialised");
    }
    return *hashes_;
  }
  
  hashes_vector& Document::getPostingHashes(){
    if (posting_hashes_==0){
      throw runtime_error("Posting Hashes not initialised");
    }
    return *posting_hashes_;
  }

  string Document::getCleanText(const uint32_t position,const uint32_t length){
    string clean_text=getText().substr(position,length);
    transform(clean_text.begin(),clean_text.end(),clean_text.begin(), ::toupper);
    replace_if(clean_text.begin(),clean_text.end(),notAlphaNumeric,' ');
    return clean_text;
  }

  string Document::getCleanText(){
    return getCleanText(0,getText().size());
  }
  
  hashes_bloom& Document::getBloom(){
    if (bloom_==0){
      throw runtime_error("Bloom not initialised");
    }
    return *bloom_;
  }

  bool Document::setText(const string& text){
    text_=new string(text);
    return permanent_?registry_->getDocumentDB()->cas(getKey().data(),getKey().size(),NULL,0,text_->data(),text_->size()):true;
  }
  
  string& Document::getText(){
    if (text_==0){
      throw runtime_error("Text not initialised");
    }
    return *text_;
  }
  
  string& Document::getKey(){
    return *key_;
  }
  
  uint32_t Document::doctype(){
    return doctype_;
  }
      
  uint32_t Document::docid(){
    return docid_;
  } 
    
  ostream& operator<< (ostream& stream, Document& document) {
    stream << "Document(" << document.doctype() << "," << document.docid() << ")";
    return stream;
  }

  void Document::fillJSONDictionary(TemplateDictionary* dict,set<string>& metadata){
    vector<string> keys;
    if (getMetaKeys(keys)){
      for (vector<string>::iterator it=keys.begin();it!=keys.end();it++){
        string value=getMeta(&(*it->c_str()));
        fillMetaDictionary(*it,value,dict,metadata);
      }
    }
  }
  
  bool operator< (Document& lhs,Document& rhs){
    if (lhs.doctype() == rhs.doctype()){
      return lhs.docid() < rhs.docid();
    } 
    return lhs.doctype() < rhs.doctype();
  }

  // -----------------------
  // DocumentManager members
  // -----------------------
  
  DocumentManager::DocumentManager(Registry* registry):
  registry_(registry){}

  DocumentManager::~DocumentManager(){}
  
  DocumentPtr DocumentManager::createTemporaryDocument(const string& content,const int32_t state){
    return createDocument(0,0,content,state,false);;
  }
  
  DocumentPtr DocumentManager::createPermanentDocument(const uint32_t doctype, const uint32_t docid,const string& content,const int32_t state){
    assert((doctype>0) && (docid>0));
    DocumentPtr doc = createDocument(doctype,docid,content,state,true);
    return doc;
  }

  bool DocumentManager::removePermanentDocument(DocumentPtr doc){
    assert((doc->doctype()>0) && (doc->docid()>0));
    return doc->remove();
  }
  
  DocumentPtr DocumentManager::getDocument(const uint32_t doctype, const uint32_t docid,const int32_t state){
    assert((doctype>0) && (docid>0));
    DocumentPtr doc(new Document(doctype,docid,true,registry_));
    if (initDoc(doc,state))
      return doc;
    return DocumentPtr();
  }
  
  DocumentPtr DocumentManager::getDocument(const string& key,const int32_t state){
    DocumentPtr doc(new Document(key,true,registry_));
    assert(initDoc(doc,state));
    return doc;
  }
  
  DocumentPtr DocumentManager::createDocument(const uint32_t doctype, const uint32_t docid,const string& content,const int32_t state,const bool permanent){
    DocumentPtr doc(new Document(doctype,docid,permanent,registry_));
    metadata_map content_map;
    kt::wwwformtomap(content,&content_map);
    if (content_map.find("text")==content_map.end()){
      return DocumentPtr();
    }
    if (not doc->setText(content_map["text"])){
      return DocumentPtr();
    }
    content_map.erase("text");
    assert(initDoc(doc,state|META));
    for(metadata_map::const_iterator it=content_map.begin(),ite=content_map.end();it!=ite;++it){
      assert(doc->setMeta(it->first,it->second));
    }
    assert(doc->setMeta("characters",toString(doc->getText().size())));
    assert(doc->setMeta("doctype",toString(doctype)));
    assert(doc->setMeta("docid",toString(docid)));
    return doc;
  }
  
  bool DocumentManager::initDoc(const DocumentPtr doc,const int32_t state){
    bool success=true;
    doc->getInstrument()->startTimer(INIT);
    if (success && (state&META) && (not doc->initMeta()))
      success=false;
    if (success && (state&TEXT) && (not doc->initText()))
      success=false;
    if (success && (state&HASHES) && (not doc->initHashes()))
      success=false;
    if (success && (state&POSTING_HASHES) && (not doc->initPostingHashes()))
      success=false;
    if (success && (state&BLOOM) && (not doc->initBloom()))
      success=false;
    doc->getInstrument()->stopTimer(INIT);
    return success;
  }
}//namespace superfastmatch
