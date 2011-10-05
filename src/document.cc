#include "document.h"

namespace superfastmatch
{
  // -----------------------
  // Document members
  // -----------------------

  Document::Document(const uint32_t doctype,const uint32_t docid, const bool permanent,Registry* registry):
  doctype_(doctype),docid_(docid),permanent_(permanent),empty_meta_(new string()),registry_(registry),key_(0),text_(0),metadata_(),hashes_(0),bloom_(0)
  {
    char key[8];
    uint32_t dt=kc::hton32(doctype_);
    uint32_t di=kc::hton32(docid_);
    memcpy(key,&dt,4);
    memcpy(key+4,&di,4);
    key_ = new string(key,8);
  }
  
  Document::Document(const string& key,const bool permanent,Registry* registry):
  permanent_(permanent),empty_meta_(new string()),registry_(registry),key_(0),text_(0),metadata_(0),hashes_(0),bloom_(0)
  {
    key_= new string(key);
    memcpy(&doctype_,key.data(),4);
    memcpy(&docid_,key.data()+4,4);
    doctype_=kc::ntoh32(doctype_);
    docid_=kc::ntoh32(docid_);
  }
  
  Document::~Document(){
    // cout << "Deleting " << *this <<endl;;
    if (metadata_!=0){
      delete metadata_;
      metadata_=0;
    }
    if (hashes_!=0){
      delete hashes_;
      hashes_=0;
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
  
  string Document::generateMetaKey(const DocumentOrder order,const string& key,const string& value){
    string meta_key;
    string padded_value=padIfNumber(value);
    switch(order){
      case DEFAULT:
        meta_key=toString(order)+getKey()+key;
        break;
      case FORWARD:
        meta_key=toString(order)+key+padded_value+getKey();
        break;
      case REVERSE:
        string reverse_value=padded_value;
        for (size_t i=0;i<reverse_value.size();i++){
          reverse_value[i]=~reverse_value[i];
        }
        meta_key=toString(order)+key+reverse_value+getKey();
        break;
    }
    // cout << key << ":" << value << ":" << meta_key << endl;
    return meta_key;
  }

  bool Document::setMeta(const string& key, const string& value){
    if (metadata_==0){
      throw runtime_error("Meta not initialised");
    }
    (*metadata_)[key]=value;
    if (permanent_){
      return registry_->getMetaDB()->set(generateMetaKey(DEFAULT,key,""),value) &&\
             registry_->getMetaDB()->set(generateMetaKey(FORWARD,key,value),"") &&\
             registry_->getMetaDB()->set(generateMetaKey(REVERSE,key,value),"");
    }
    return true;
  }

  
  bool Document::initMeta(){
    if (metadata_==0){
      metadata_map* tempMap = new metadata_map();
      vector<string> meta_keys;
      string meta_value;
      registry_->getMetaDB()->match_prefix(generateMetaKey(DEFAULT,"",""),&meta_keys);
      for (vector<string>::const_iterator it=meta_keys.begin(),ite=meta_keys.end();it!=ite;++it){
        if (not registry_->getMetaDB()->get(*it,&meta_value)){
          return false;
        };
        (*tempMap)[(*it).substr(9)]=meta_value;
      }      
      metadata_=tempMap;
    }
    return true;
  }

  bool Document::initText(){
    if (text_==0){
      text_ = new string();
      return registry_->getDocumentDB()->get(getKey(),text_);
    }
    return true;
  }

  bool Document::initHashes(){
    if (hashes_==0){
      hashes_vector* tempHashes = new hashes_vector();
      if(getText().size()>=registry_->getWindowSize()){
        UpperCaseRabinKarp(getText(),registry_->getWindowSize(),registry_->getWhiteSpaceThreshold(),*tempHashes);
      }
      hashes_=tempHashes;
    }
    return true;
  }

  bool Document::initBloom(){
    if (bloom_==0){
      hashes_bloom* tempBloom = new hashes_bloom();
      for (hashes_vector::const_iterator it=getHashes().begin(),ite=getHashes().end();it!=ite;++it){
        tempBloom->set((*it)&0x3FFFFFF);
      }
      bloom_=tempBloom;
    }
    return true;
  }
  
  bool Document::remove(){
    bool success=registry_->getDocumentDB()->remove(*key_);
    vector<string> db_keys;
    vector<string> meta_keys;
    assert(getMetaKeys(meta_keys));
    for (vector<string>::iterator it=meta_keys.begin(),ite=meta_keys.end();it!=ite;++it){
      db_keys.push_back(generateMetaKey(DEFAULT,*it,getMeta(*it)));
      db_keys.push_back(generateMetaKey(FORWARD,*it,getMeta(*it)));
      db_keys.push_back(generateMetaKey(REVERSE,*it,getMeta(*it)));
    }
    success = success && (registry_->getMetaDB()->remove_bulk(db_keys)!=-1);
    return success;
  }
  
  hashes_vector& Document::getHashes(){
    if (hashes_==0){
      throw runtime_error("Hashes not initialised");
    }
    return *hashes_;
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
  
  void Document::fill_document_dictionary(TemplateDictionary* dict){
    vector<string> keys;
    if (getMetaKeys(keys)){
      for (vector<string>::iterator it=keys.begin();it!=keys.end();it++){
        TemplateDictionary* meta_dict=dict->AddSectionDictionary("META");
        meta_dict->SetValue("KEY",*it);
        meta_dict->SetValue("VALUE",getMeta(&(*it->c_str())));
      } 
    }
    dict->SetValue("TEXT",getText());
    registry_->getAssociationManager()->fillListDictionary(shared_from_this(),dict);
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
  
  bool stripMeta(string lhs,string rhs){
    return lhs.substr(1,8).compare(rhs.substr(1,8))==0;
  }
  
  vector<DocumentPtr> DocumentManager::getDocuments(const uint32_t doctype,const int32_t state){
    vector<DocumentPtr> documents;
    vector<string> keys;
    if (doctype!=0){
      char key[4];
      uint32_t dt=kc::hton32(doctype);
      memcpy(key,&dt,4);
      string match(key,4);
      registry_->getMetaDB()->match_prefix(toString(Document::DEFAULT)+match,&keys);
    }else{
      registry_->getMetaDB()->match_prefix(toString(Document::DEFAULT),&keys);
    }
    vector<string>::iterator end=std::unique(keys.begin(),keys.end(),stripMeta);
    keys.resize(end-keys.begin());
    for(vector<string>::iterator it=keys.begin(),ite=keys.end();it!=ite;++it){
      documents.push_back(getDocument((*it).substr(1,8),state));
    }
    return documents;
  }
  
  DocumentPtr DocumentManager::createDocument(const uint32_t doctype, const uint32_t docid,const string& content,const int32_t state,const bool permanent){
    DocumentPtr doc(new Document(doctype,docid,permanent,registry_));
    metadata_map content_map;
    kt::wwwformtomap(content,&content_map);
    if (not doc->setText(content_map["text"])){
      return DocumentPtr();
    }
    content_map.erase("text");
    assert(initDoc(doc,state|META));
    assert(doc->setMeta("characters",toString(doc->getText().size())));
    for(metadata_map::const_iterator it=content_map.begin(),ite=content_map.end();it!=ite;++it){
      assert(doc->setMeta(it->first,it->second));
    }
    return doc;
  }
  
  bool DocumentManager::initDoc(const DocumentPtr doc,const int32_t state){
    if ((state&META) && (not doc->initMeta()))
      return false;
    if ((state&TEXT) && (not doc->initText()))
      return false;
    if ((state&HASHES) && (not doc->initHashes()))
      return false;
    if ((state&BLOOM) && (not doc->initBloom()))
      return false;
    return true;
  }
  
  // -----------------------
  // DocumentCursor members
  // -----------------------
  
  // DocumentCursor::DocumentCursor(Registry* registry,const string& meta_key,const Document::DocumentOrder order):
  // registry_(registry),
  // order_(order),
  // meta_key_(meta_key),
  // previous_key_("0",9)
  // {
  //   assert(meta_key==""||order!=Document::DEFAULT);
  //   cursor_=registry->getMetaDB()->cursor();
  //   cursor_->jump(toString(order)+meta_key);
  // };
  // 
  // DocumentCursor::~DocumentCursor(){
  //   delete cursor_;
  // }
  // 
  // bool DocumentCursor::jumpFirst(){
  //   return cursor_->jump();
  // }
  // 
  // bool DocumentCursor::jumpLast(){
  //   return cursor_->jump_back();
  // };
  // 
  // bool DocumentCursor::jump(string& key){
  //   return cursor_->jump(key);
  // };
  // 
  // DocumentPtr DocumentCursor::getNext(const int32_t state){
  //   string key;
  //   while(cursor_->get_key(&key,true)&&(key.compare(0,1,toString(order_))==0)){
  //     if ((order_==Document::DEFAULT)&&(key.substr(1,8)!=previous_key_.substr(1,8))){
  //       previous_key_=key;
  //       return registry_->getDocumentManager()->getDocument(key.substr(1,8),state);
  //     }else if((order_!=Document::DEFAULT)&&(key.compare(1,meta_key_.size(),meta_key_)==0)){
  //       return registry_->getDocumentManager()->getDocument(key.substr(key.size()-8),state);
  //     }
  //   }
  //   return DocumentPtr();
  // };
  // 
  // DocumentPtr DocumentCursor::getPrevious(){
  //   return DocumentPtr();
  // };
  // 
  // uint32_t DocumentCursor::getCount(){
  //   return registry_->getDocumentDB()->count();
  // };
}//namespace superfastmatch
