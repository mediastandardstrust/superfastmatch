#include <query.h>
#include <document.h>

namespace superfastmatch
{
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
  
  DocumentQuery::DocumentQuery(Registry* registry, const string& command):
  registry_(registry),
  desc_(false),
  valid_(true)
  {
    map<string,string> elements,parameters;
    vector<string> paths,sections,parameter;
    urlbreak(command.c_str(),&elements);
    path_=elements["path"];
    kc::strsplit(path_, '/', &paths);
    if (paths.size()>3){
      valid_&=target_.parse(paths[3]);
    }
    if(paths.size()>2){
      valid_&=source_.parse(paths[2]);
    }
    kc::strsplit(elements["query"],'&',&sections);
    for(vector<string>::iterator it=sections.begin(),ite=sections.end();it!=ite;++it){
      kc::strsplit(*it,'=',&parameter);
      if(parameter.size()==2){
        size_t ksiz,vsiz;
        char* kbuf = kc::urldecode(parameter[0].c_str(), &ksiz);
        char* vbuf = kc::urldecode(parameter[1].c_str(), &vsiz);
        parameters[kbuf]=vbuf;
        delete[] kbuf;
        delete[] vbuf;
      }
    }
    cursor_=parameters["cursor"];
    if (parameters.find("order_by")!=parameters.end()){
      if (parameters["order_by"][0]=='-'){
        order_by_=parameters["order_by"].substr(1);
        desc_=true;
      }else{
        order_by_=parameters["order_by"];
        desc_=false;
      }
    }
    limit_=kc::atoi(parameters["limit"].c_str());
    if (limit_==0){
      limit_=registry->getPageSize();
    }
  }

  const bool DocumentQuery::isValid(){
    return valid_;
  }
  
  const string& DocumentQuery::getCursor() const{
    return cursor_;
  }
  
  const string& DocumentQuery::getOrder() const{
    return order_by_;
  }
  
  const bool DocumentQuery::isDescending() const{
    return desc_;
  }
  
  const uint64_t DocumentQuery::getLimit() const{
    return limit_;
  }

  const vector<DocPair>& DocumentQuery::getSourceDocPairs(){
    if (source_pairs_.size()==0){
      source_pairs_=getDocPairs(source_,order_by_,cursor_,limit_,desc_);
    }
    return source_pairs_;
  }
  
  const vector<DocPair>& DocumentQuery::getTargetDocPairs(){
    if (target_pairs_.size()==0){
      target_pairs_=getDocPairs(target_,order_by_,cursor_,limit_,desc_);
    }
    return target_pairs_;
  }
  
  const string& DocumentQuery::getFirst(){
    if (first_.empty()){
      first_=getCommand();
    }
    return first_;
  }
  
  const string& DocumentQuery::getLast(){
    if (last_.empty()){
      last_=getCommand(getDocPairs(source_,order_by_,"",limit_,!desc_).back());
    }
    return last_;
  }
  
  const string& DocumentQuery::getNext(){
    if (next_.empty()){
      string cursor=getCursor(getSourceDocPairs().back());
      vector<DocPair> pairs=getDocPairs(source_,order_by_,cursor,1,desc_);
      if (pairs.size()==1){
        next_=getCommand(pairs.back());
      }else{
        next_=getLast();
      }
    }
    return next_;
  }
  
  const string& DocumentQuery::getPrevious(){
    if (previous_.empty()){
      string cursor=getCursor(getSourceDocPairs().front());
      vector<DocPair> pairs=getDocPairs(source_,order_by_,cursor,1,!desc_);
      if (pairs.size()==1){
        previous_=getCommand(pairs.front());
      }else{
        previous_=getFirst();
      }
    }
    return previous_;
  }
  
  const string DocumentQuery::getCursor(const DocPair& pair)const{
    DocumentPtr doc=registry_->getDocumentManager()->getDocument(pair.doc_type,pair.doc_id,DocumentManager::META);
    string key = doc->getMeta(order_by_)+doc->getKey();
    return string(kc::urlencode(key.c_str(), key.size()));
  }
  
  const string DocumentQuery::getCommand(const DocPair& pair) const{
    return getCommand()+"&cursor="+getCursor(pair);
  }
  
  const string DocumentQuery::getCommand() const{
    string command(path_+"?limit="+toString(limit_));
    if (!order_by_.empty()){
      command+="&order_by=";
      if(desc_){
        command+="-"; 
      }
      command+=order_by_;
    }
    return command;
  }
  
  vector<DocPair> DocumentQuery::getDocPairs(const DocTypeRange& range, const string& order_by,const string& cursor, const uint64_t limit, const bool desc) const{
    vector<DocPair> pairs;
    kc::PolyDB::Cursor* cur=registry_->getMetaDB()->cursor();
    string key;
    uint64_t count=0;
    if (order_by.empty()){
      if (not cur->jump(cursor)){
        if (desc){
          cur->jump_back();
        }else{
          cur->jump();
        }
      }
      string previous_key="dUmMyKeY";
      while((cur->get_key(&key))&&(key.compare(0,1,"0")==0)&&(count<limit)){
        if (key.substr(1,8)!=previous_key.substr(1,8)){
          DocPair pair(key.substr(1,8));
          // cout << pair.doc_type << ":" << pair.doc_id << ":" << range.isInRange(pair.doc_type) << endl;
          if (range.isInRange(pair.doc_type)){
            pairs.push_back(pair);
            count++;
          }
        }
        if (desc){
          if (not cur->step_back()){
            break;
          }
        }else if(not cur->step()){
          break;
        }
        previous_key=key;
      }
    }else{
      string match="1"+order_by;
      string start=match+cursor;
      if (desc){
        for (size_t i=start.size();i>0;i--){
          if (start[i-1]!=255){
            start.replace(i-1,1,1,start[i-1]+1);
            break;
          }
        }
        assert(cur->jump_back(start));
      }else{
        assert(cur->jump(start));
        // cout << start << endl;
      }
      while((cur->get_key(&key))&&(key.compare(0,match.size(),match)==0)&&(count<limit)){
        // cout << count << ":" << key << endl;
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
    }
    delete cur;
    return pairs;
  } 
}
