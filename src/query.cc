#include <query.h>

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
  
  set<uint32_t> DocTypeRange::getDocTypes(){
    return doctypes_;
  }
  
  DocumentQuery::DocumentQuery(Registry* registry, const string& command):
  registry_(registry),
  valid_(true)
  {
    vector<string> parts;
    kc::strsplit(command, '/', &parts);
    if (parts.size()>0){
      valid_&=source_.parse(parts[0]);
    }
    if (parts.size()>1){
      valid_&=target_.parse(parts[1]); 
    }
  }

  bool DocumentQuery::isValid(){
    return valid_;
  }

  vector<DocPair> DocumentQuery::getSourceDocPairs(){
    return getDocPairs(source_);
  }
  
  vector<DocPair> DocumentQuery::getTargetDocPairs(){
    return getDocPairs(target_);
  }
  
  vector<DocPair> DocumentQuery::getDocPairs(const DocTypeRange& range){
    kc::PolyDB::Cursor* cursor=registry_->getMetaDB()->cursor();
    vector<DocPair> pairs;
    cursor->jump();
    delete cursor;
    return pairs;
  }
  
}
