#include "document.h"

namespace superfastmatch
{
  
  // -----------------------
  // DocumentCursor members
  // -----------------------
  
  DocumentCursor::DocumentCursor(Registry* registry):
  registry_(registry){
    cursor_=registry->getDocumentDB()->cursor();
    cursor_->jump();
  };

  DocumentCursor::~DocumentCursor(){
    delete cursor_;
  }

  bool DocumentCursor::jumpFirst(){
    return cursor_->jump();
  }

  bool DocumentCursor::jumpLast(){
    return cursor_->jump_back();
  };

  bool DocumentCursor::jump(string& key){
    return cursor_->jump(key);
  };

  Document* DocumentCursor::getNext(){
    string key;
    if (cursor_->get_key(&key,true)){
      Document* doc = new Document(key,registry_);
      return doc;
    };
    return NULL;
  };

  Document* DocumentCursor::getPrevious(){
    return NULL;
  };

  uint32_t DocumentCursor::getCount(){
    return registry_->getDocumentDB()->count();
  };

  struct MetaKeyComparator {
    bool operator() (const string& lhs, const string& rhs) const{
      if (lhs==rhs)
        return false;
      if ((lhs=="title"))
        return true;
      if ((lhs=="characters") && (rhs=="title"))
        return false;
      if (lhs=="characters")
        return true;
      return lhs<rhs;
    }
  };

  void DocumentCursor::fill_list_dictionary(TemplateDictionary* dict,uint32_t doctype,uint32_t docid){
    if (getCount()==0){
      return;
    }
    TemplateDictionary* page_dict=dict->AddIncludeDictionary("PAGING");
    page_dict->SetFilename(PAGING);
    uint32_t count=0;
    Document* doc;
    char* key=new char[8];
    size_t key_length;
    uint32_t di;
    if (doctype!=0){
      uint32_t dt=kc::hton32(doctype);
      memcpy(key,&dt,4);
      cursor_->jump(key,4);
    }
    delete[] key;
    key=cursor_->get_key(&key_length,false);
    if (key!=NULL){
      memcpy(&di,key+4,4);
      di=kc::ntoh32(di);
      page_dict->SetValueAndShowSection("PAGE",toString(di),"FIRST");
    }
    if (docid!=0){
      di=kc::hton32(docid);
      memcpy(key+4,&di,4);
      cursor_->jump(key,8);
    }
    delete[] key;
    vector<Document*> docs;
    vector<string> keys;
    set<string,MetaKeyComparator> keys_set;
    while (((doc=getNext())!=NULL)&&(count<registry_->getPageSize())){
      if ((doctype!=0) && (doctype!=doc->doctype())){
        break;
      }
      docs.push_back(doc);
      if (doc->getMetaKeys(keys)){
        for (vector<string>::iterator it=keys.begin();it!=keys.end();it++){
          keys_set.insert(*it);
        } 
      }
      count++;
    }
    for (set<string>::const_iterator it=keys_set.begin(),ite=keys_set.end();it!=ite;++it){
      TemplateDictionary* keys_dict=dict->AddSectionDictionary("KEYS");
      keys_dict->SetValue("KEY",*it);
    }
    for (vector<Document*>::iterator it=docs.begin(),ite=docs.end();it!=ite;++it){
      TemplateDictionary* doc_dict = dict->AddSectionDictionary("DOCUMENT");
      doc_dict->SetIntValue("DOC_TYPE",(*it)->doctype());
      doc_dict->SetIntValue("DOC_ID",(*it)->docid()); 
      for (set<string>::const_iterator it2=keys_set.begin(),ite2=keys_set.end();it2!=ite2;++it2){
        TemplateDictionary* values_dict=doc_dict->AddSectionDictionary("VALUES");
        values_dict->SetValue("VALUE",(*it)->getMeta(&(*it2->c_str())));
      }
    }
  
    if (doc!=NULL){
      key=cursor_->get_key(&key_length,false);
      memcpy(&di,key+4,4);
      di=kc::ntoh32(di);
      page_dict->SetValueAndShowSection("PAGE",toString(di),"NEXT");
      delete[] key;
    }
    if ((doctype==0)&&(cursor_->jump_back())){
      key=cursor_->get_key(&key_length,false);
      memcpy(&di,key+4,4);
      di=kc::ntoh32(di);
      page_dict->SetValueAndShowSection("PAGE",toString(di),"LAST");
      delete[] key;
    }
    FreeClear(docs);
  }

  // -----------------------
  // Document members
  // -----------------------

  Document::Document(const uint32_t doctype,const uint32_t docid,const string& content,Registry* registry):
  doctype_(doctype),docid_(docid),empty_meta_(new string()),registry_(registry),key_(0),text_(0),clean_text_(0),metadata_(new metadata_map()),hashes_(0),bloom_(0)
  {
    char key[8];
    uint32_t dt=kc::hton32(doctype_);
    uint32_t di=kc::hton32(docid_);
    memcpy(key,&dt,4);
    memcpy(key+4,&di,4);
    key_ = new string(key,8);
    kt::wwwformtomap(content,metadata_);
    text_ = new string((*metadata_)["text"]);
    metadata_->erase("text");
    setMeta("characters",toString(text_->size()).c_str());
  }
  
  Document::Document(const uint32_t doctype,const uint32_t docid,Registry* registry):
  doctype_(doctype),docid_(docid),empty_meta_(new string()),registry_(registry),key_(0),text_(0),clean_text_(0),metadata_(new metadata_map()),hashes_(0),bloom_(0)
  {
    char key[8];
    uint32_t dt=kc::hton32(doctype_);
    uint32_t di=kc::hton32(docid_);
    memcpy(key,&dt,4);
    memcpy(key+4,&di,4);
    key_ = new string(key,8);
    text_ = new string();
    load();
  }
  
  Document::Document(string& key,Registry* registry):
  empty_meta_(new string()),registry_(registry),key_(0),text_(0),clean_text_(0),metadata_(new metadata_map()),hashes_(0),bloom_(0)
  {
    key_= new string(key);
    memcpy(&doctype_,key.data(),4);
    memcpy(&docid_,key.data()+4,4);
    doctype_=kc::ntoh32(doctype_);
    docid_=kc::ntoh32(docid_);
    text_ = new string();
    load();
  }
  
  Document::~Document(){
    if (clean_text_!=0){
      delete clean_text_;
      clean_text_=0;
    }
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
  
  bool Document::initMeta(){
    if (metadata_==0){
      metadata_map* tempMap = new metadata_map();
      vector<string> meta_keys;
      string meta_value;
      registry_->getMetaDB()->match_prefix(getKey(),&meta_keys);
      for (vector<string>::const_iterator it=meta_keys.begin(),ite=meta_keys.end();it!=ite;++it){
        if (not registry_->getMetaDB()->get(*it,&meta_value)){
          return false;
        };
        (*tempMap)[(*it).substr(8)]=meta_value;
      }      
      metadata_=tempMap;
    }
    return true;
  }

  bool Document::initText(){
    if (text_==0){
      return registry_->getDocumentDB()->get(*key_,text_);
    }
    return true;
  }
  
  bool Document::initCleanText(){
    if (clean_text_==0){
      string* tempClean=new string(getText().size(),' ');
      replace_copy_if(getText().begin(),getText().end(),tempClean->begin(),notAlphaNumeric,' ');
      transform(tempClean->begin(), tempClean->end(), tempClean->begin(), ::tolower);
      clean_text_=tempClean;
    }
    return true;
  }

  bool Document::initHashes(){
    if (hashes_==0){
      hashes_vector* tempHashes = new hashes_vector();
      uint32_t length = getCleanText().length()-registry_->getWindowSize();
      hash_t white_space = registry_->getWhiteSpaceHash(false);
      uint32_t window_size=registry_->getWindowSize();
      uint32_t white_space_threshold=registry_->getWhiteSpaceThreshold();
      tempHashes->resize(length);
      hash_t hash;
      uint32_t i=0;
      string::const_iterator it=getCleanText().begin(),ite=getCleanText().end()-registry_->getWindowSize();
      for (;it!=ite;++it){
        // if ((count(it,it+white_space_threshold,' ')+count(it+window_size-white_space_threshold,it+window_size,' '))>white_space_threshold){
        if (count(it,it+window_size,' ')>white_space_threshold){
          hash=white_space;
        }else{
          hash=hashmurmur(&(*it),window_size+1);
        }
        (*tempHashes)[i]=hash;
        i++;
      }
      hashes_=tempHashes;
    }
    return true;
  }
  
  bool Document::initBloom(){
    if (bloom_==0){
      hashes_bloom* tempBloom = new hashes_bloom();
      for (hashes_vector::const_iterator it=hashes().begin(),ite=hashes().end();it!=ite;++it){
        tempBloom->set(*it&0xFFFFFF);
      }
      bloom_=tempBloom;
    }
    return true;
  }

  bool Document::load(){
    vector<string> meta_keys;
    string meta_value;
    registry_->getMetaDB()->match_prefix(getKey(),&meta_keys);
    for (vector<string>::const_iterator it=meta_keys.begin(),ite=meta_keys.end();it!=ite;++it){
      if (not registry_->getMetaDB()->get(*it,&meta_value)){
        return false;
      };
      (*metadata_)[(*it).substr(8)]=meta_value;
    }
    return registry_->getDocumentDB()->get(*key_,text_);
  }
  
  bool Document::save(){
    bool success = registry_->getDocumentDB()->cas(key_->data(),key_->size(),NULL,0,text_->data(),text_->size());
    for (metadata_map::const_iterator it=metadata_->begin(),ite=metadata_->end();it!=ite;++it){
      string meta_key(getKey()+it->first);
      success = success && registry_->getMetaDB()->cas(meta_key.data(),meta_key.size(),NULL,0,it->second.data(),it->second.size());
    }
    return success;
  }
  
  bool Document::remove(){
    bool success=registry_->getDocumentDB()->remove(*key_);
    vector<string> meta_keys;
    registry_->getMetaDB()->match_prefix(getKey(),&meta_keys);
    for (vector<string>::const_iterator it=meta_keys.begin(),ite=meta_keys.end();it!=ite;++it){
      success = success && registry_->getMetaDB()->remove(*it);
    }
    return success;
  }
  
  Document::hashes_vector& Document::hashes(){
    if (hashes_==0){
      hashes_ = new hashes_vector();
      if (bloom_==0){
        bloom_ = new hashes_bloom();
      }
      uint32_t length = getCleanText().length()-registry_->getWindowSize();
      hash_t white_space = registry_->getWhiteSpaceHash(false);
      uint32_t window_size=registry_->getWindowSize();
      uint32_t white_space_threshold=registry_->getWhiteSpaceThreshold();
      hashes_->resize(length);
      hash_t hash;
      uint32_t i=0;
      string::const_iterator it=getCleanText().begin(),ite=getCleanText().end()-registry_->getWindowSize();
      for (;it!=ite;++it){
        // if ((count(it,it+white_space_threshold,' ')+count(it+window_size-white_space_threshold,it+window_size,' '))>white_space_threshold){
        if (count(it,it+window_size,' ')>white_space_threshold){
          hash=white_space;
        }else{
          hash=hashmurmur(&(*it),window_size+1);
        }
        (*hashes_)[i]=hash;
        bloom_->set(hash&0xFFFFFF);
        i++;
      }
    }
    return *hashes_;
  }

  string& Document::getCleanText(){
    if (clean_text_==0){
      clean_text_=new string(getText().size(),' ');
      replace_copy_if(getText().begin(),getText().end(),clean_text_->begin(),notAlphaNumeric,' ');
      transform(clean_text_->begin(), clean_text_->end(), clean_text_->begin(), ::tolower);
    }
    return *clean_text_;
  }
  
  Document::hashes_bloom& Document::bloom(){
    if (bloom_==0){
      hashes(); 
    }
    return *bloom_;
  }

  string& Document::getMeta(const char* key){
    if ((metadata_!=0) && (metadata_->find(key)!=metadata_->end())){
      return (*metadata_)[key];
    }
    return *empty_meta_;
  }
  
  bool Document::setMeta(const char* key, const char* value){
    if (metadata_!=0){
      (*metadata_)[key]=value;
      return true;
    }
    return false;
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
  
  string& Document::getText(){
    return *text_;
  }
  
  string& Document::getKey(){
    return *key_;
  }
  
  const uint32_t Document::doctype(){
    return doctype_;
  }
      
  const uint32_t Document::docid(){
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
    TemplateDictionary* association_dict=dict->AddIncludeDictionary("ASSOCIATION");
    association_dict->SetFilename(ASSOCIATION);
    // This belongs in Association Cursor
    // And need a key based Association Constructor
    kc::PolyDB::Cursor* cursor=registry_->getAssociationDB()->cursor();
    cursor->jump(getKey().data(),8);
    string next;
    string other_key;
    while((cursor->get_key(&next,true))&&(getKey().compare(next.substr(0,8))==0)){
      other_key=next.substr(8,8);
      Document other(other_key,registry_);
      Association association(registry_,this,&other);
      association.fill_item_dictionary(association_dict);
    }
    delete cursor;
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
    DocumentPtr doc(new Document(0,0,content,registry_));
    initDoc(doc,state);
    return doc;
  }
  
  DocumentPtr DocumentManager::createPermanentDocument(const uint32_t doctype, const uint32_t docid,const string& content,const int32_t state){
    assert((doctype>0) && (docid>0));
    DocumentPtr doc(new Document(doctype,docid,content,registry_));
    initDoc(doc,state);
    doc->save();
    return doc;
  }

  DocumentPtr DocumentManager::getDocument(const uint32_t doctype, const uint32_t docid,const int32_t state){
    assert((doctype>0) && (docid>0));
    DocumentPtr doc(new Document(doctype,docid,registry_));
    doc->load();
    initDoc(doc,state);
    return doc;
  }
  
  bool DocumentManager::initDoc(const DocumentPtr doc,const int32_t state){
    bool success=true;
    if (state&META)
      success&=doc->initMeta();
    if (state&TEXT)
      success&=doc->initText();
    if (state&CLEAN_TEXT)
      success&=doc->initCleanText();
    if (state&HASHES)
      success&=doc->initHashes();
    if (state&BLOOM)
      success&=doc->initBloom();
    return success;
  }

}//namespace superfastmatch
