#include <query.h>
#include <document.h>

namespace superfastmatch
{
  RegisterTemplateFilename(QUERY_JSON, "JSON/query.tpl");
  RegisterTemplateFilename(DOCUMENTS_JSON, "JSON/documents.tpl");

  ostream& operator<<(ostream &stream,const DocPair& pair){
    stream << "Document("<< pair.doc_type << "," << pair.doc_id <<")";
    return stream;
  }

  DocTypeRange::DocTypeRange(){}
  
  bool DocTypeRange::parse(const string& range){
    if (range.empty()){
      return true;
    }
    bool valid=true;
    istringstream iss(range);
    string section;
    while(!iss.eof()){
      getline(iss,section,';');
      size_t splits = count(section.begin(),section.end(),'-');
      if (splits>1){
        valid=false;
        continue;
      }else if (splits==1){
        size_t split=section.find("-");
        uint32_t start=kc::atoi(section.substr(0,split).c_str());
        uint32_t end=kc::atoi(section.substr(split+1).c_str());
        start = min(start,end);
        end = max(start,end);
        if (start!=0 && end!=0){
          for (uint32_t i=start;i<=end;i++){
            doctypes_.insert(i);
          } 
        }else{
          valid=false;
        }
      }else{
        uint32_t single=kc::atoi(section.c_str());
        if (single!=0){
          doctypes_.insert(single);
        }else{
          valid=false;
        }
      }
    }
    if (!valid){
      doctypes_.clear();
    }
    return valid;
  }
  
  bool DocTypeRange::isInRange(uint32_t doctype) const{
    return (doctypes_.size()==0 || doctypes_.find(doctype)!=doctypes_.end());
  }
  
  set<uint32_t> DocTypeRange::getDocTypes(){
    return doctypes_;
  }
  
  DocumentQuery::DocumentQuery(Registry* registry):
  registry_(registry),
  order_by_("doctype"),
  limit_(registry->getPageSize()),
  desc_(false),
  valid_(true)
  {}
  
  
  DocumentQuery::DocumentQuery(Registry* registry,const string& source,const string& target, const map<string,string>& query):
  registry_(registry),
  order_by_("doctype"),
  limit_(registry->getPageSize()),
  desc_(false),
  valid_(true)
  {
    if (source.size()>0){
      valid_&=source_.parse(source);
    }
    if (target.size()>0){
      valid_&=target_.parse(target);
    }
    map<string,string>::const_iterator cursor=query.find("cursor");
    if (cursor!=query.end()){
      cursor_=cursor->second;
    }
    map<string,string>::const_iterator order_by=query.find("order_by");
    if (order_by!=query.end()){
      if (order_by->second[0]=='-'){
        order_by_=order_by->second.substr(1);
        desc_=true;
      }else{
        order_by_=order_by->second;
        desc_=false;
      }
    }
    map<string,string>::const_iterator limit=query.find("limit");
    if(limit!=query.end()){
      limit_=kc::atoi(limit->second.c_str());      
    }
  }
  
  bool DocumentQuery::isValid(){
    return valid_;
  }
  
  const string& DocumentQuery::getCursor() const{
    return cursor_;
  }
  
  const string& DocumentQuery::getOrder() const{
    return order_by_;
  }
  
  bool DocumentQuery::isDescending(){
    return desc_;
  }
  
  uint64_t DocumentQuery::getLimit(){
    return limit_;
  }

  const vector<DocPair>& DocumentQuery::getSourceDocPairs(bool unlimited){
    if (source_pairs_.size()==0){
      source_pairs_=getDocPairs(source_,order_by_,cursor_,unlimited?numeric_limits<uint64_t>::max():limit_,desc_);
    }
    return source_pairs_;
  }
  
  const vector<DocPair>& DocumentQuery::getTargetDocPairs(bool unlimited){
    if (target_pairs_.size()==0){
      target_pairs_=getDocPairs(target_,order_by_,cursor_,unlimited?numeric_limits<uint64_t>::max():limit_,desc_);
    }
    return target_pairs_;
  }
  
  const string& DocumentQuery::getFirst(){
    if (first_.empty()){
      first_="";
    }
    return first_;
  }
  
  const string& DocumentQuery::getLast(){
    if (last_.empty()){
      vector<DocPair> pairs=getDocPairs(source_,order_by_,"",limit_,!desc_);
      if (pairs.size()>0){
        last_=getCommand(getDocPairs(source_,order_by_,"",limit_,!desc_).back()); 
      }
    }
    return last_;
  }
  
  const string& DocumentQuery::getNext(){
    if (next_.empty() && getSourceDocPairs().size()>0){
      string cursor=getCursor(getSourceDocPairs().back());
      vector<DocPair> pairs=getDocPairs(source_,order_by_,cursor,2,desc_);
      if (pairs.size()>0){
        next_=getCommand(pairs.back());
      }else{
        next_=getLast();
      }
    }
    return next_;
  }
  
  const string& DocumentQuery::getPrevious(){
    if (previous_.empty() && getSourceDocPairs().size()>0){
      string cursor=getCursor(getSourceDocPairs().front());
      vector<DocPair> pairs=getDocPairs(source_,order_by_,cursor,limit_+2,!desc_);
      if (pairs.size()>0){
        previous_=getCommand(pairs.back());
      }else{
        previous_=getFirst();
      }
    }
    return previous_;
  }
  
  const string DocumentQuery::getCursor(const DocPair& pair)const{
    DocumentPtr doc=registry_->getDocumentManager()->getDocument(pair.doc_type,pair.doc_id,DocumentManager::META);
    string cursor;
    if (doc){
      cursor=doc->getMeta(order_by_);
      char* encoded=kc::urlencode(cursor.c_str(), cursor.size());
      cursor=encoded;
      delete[] encoded;
      cursor+=":"+toString(doc->doctype())+":"+toString(doc->docid());
    }
    return cursor;
  }
  
  const string DocumentQuery::parseCursor(const string& cursor)const{
    size_t splits = count(cursor.begin(),cursor.end(),':');
    if (splits!=2){
      return cursor;
    }
    size_t csiz;
    size_t first=cursor.find(":");
    size_t second=cursor.find(":",first+1);
    string parsed=cursor.substr(0,first);
    char* decoded = kc::urldecode(parsed.c_str(),&csiz);
    parsed=decoded;
    delete[] decoded;
    parsed=padIfNumber(parsed);
    uint32_t doctype=kc::atoi(cursor.substr(first+1,second).c_str());
    if (doctype!=0){
      char key[4];
      doctype=kc::hton32(doctype);
      memcpy(key,&doctype,4);
      parsed+=string(key,4);
    }
    uint32_t docid=kc::atoi(cursor.substr(second+1).c_str());
    if (docid!=0){
      char key[4];
      docid=kc::hton32(docid);
      memcpy(key,&docid,4);
      parsed+=string(key,4);
    }
    return parsed;
  }
  
  const string DocumentQuery::getCommand(const DocPair& pair) const{
    return getCursor(pair);
  }
    
  vector<DocPair> DocumentQuery::getDocPairs(const DocTypeRange& range, const string& order_by,const string& cursor, const uint64_t limit, const bool desc) const{
    kc::PolyDB::Cursor* cur=registry_->getOrderedMetaDB()->cursor();
    vector<DocPair> pairs;
    string key;
    uint64_t count=0;
    string start=(cursor.size()==0)?order_by:(order_by+parseCursor(cursor));
    if (desc){
      for (size_t i=start.size();i>0;i--){
        if (start[i-1]!=255){
          start.replace(i-1,1,1,start[i-1]+1);
          break;
        }
      }
      if (not cur->jump_back(start)){
        cur->jump_back();
      }
    }else{
      if (not cur->jump(start)){
        cur->jump();
      }
    }
    while((cur->get_key(&key))&&(key.compare(0,order_by.size(),order_by)==0)&&(count<limit)){
      DocPair pair(key.substr(key.size()-8));
      if (range.isInRange(pair.doc_type)){
        pairs.push_back(pair);
        count++;
      }
      if (desc){
        if (not cur->step_back()){
          break;
        }
      }else if(not cur->step()){
        break;
      }
    }
    delete cur;
    return pairs;
  } 
  
  void DocumentQuery::fillJSONDictionary(TemplateDictionary* dict){
    TemplateDictionary* queryDict=dict->AddIncludeDictionary("DATA");
    queryDict->SetFilename(QUERY_JSON);
    TemplateDictionary* documentsDict=queryDict->AddIncludeDictionary("DOCUMENTS");
    documentsDict->SetFilename(DOCUMENTS_JSON);
    set<string> metadata;
    for(vector<DocPair>::const_iterator it=getSourceDocPairs().begin(),ite=getSourceDocPairs().end();it!=ite;++it){
      DocumentPtr doc=registry_->getDocumentManager()->getDocument(it->doc_type,it->doc_id,DocumentManager::META);
      TemplateDictionary* docDict = documentsDict->AddSectionDictionary("DOCUMENT");
      doc->fillJSONDictionary(docDict,metadata);
    }
    for (set<string>::const_iterator it=metadata.begin(),ite=metadata.end();it!=ite;++it){
      TemplateDictionary* fields_dict=documentsDict->AddSectionDictionary("FIELDS");
      fields_dict->SetValue("FIELD",*it);
    }
    queryDict->SetIntValue("TOTAL",registry_->getDocumentDB()->count());
    queryDict->SetValue("CURRENT",getCursor());
    queryDict->SetValue("FIRST",getFirst());
    queryDict->SetValue("LAST",getLast());
    queryDict->SetValue("PREVIOUS",getPrevious());
    queryDict->SetValue("NEXT",getNext());
  }
}
