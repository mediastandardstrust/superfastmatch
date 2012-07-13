#include <loader.h>

namespace superfastmatch{

  Loader::Loader(Logger* logger, const string& url, const string& source):
  url_(url),
  source_(source),
  attempts_(0),
  logger_(logger)
  {
    if (url_.authority().size()>0){
      char* auth = kc::baseencode(url_.authority().data(),url_.authority().size());
      headers_["Authorization"]="Basic ";
      headers_["Authorization"]+=auth;
      delete[] auth;
    }
  }

  Loader::~Loader(){
    client_.close();
  }

  bool Loader::getUrl(const string& path,string& body){
    bool success=false;
    if (not open_){
      open_=client_.open(url_.host(),url_.port());      
    }
    if (open_){
      success=(client_.fetch(path,kt::HTTPClient::MGET,&body,NULL,NULL,&headers_)!=-1);
      string message;
      if (not success && attempts_<5){
        message=kc::strprintf("Failed to retrieve: %s%s",url_.expression().c_str(),path.c_str());
        attempts_++;
        logger_->log(Logger::ERROR,message.c_str());
        open_=client_.close();
        return getUrl(path,body);
      }else{
        message=kc::strprintf("Successfully retrieved: %s%s",url_.expression().c_str(),path.c_str());
        logger_->log(Logger::DEBUG,message.c_str());
        attempts_=0;
      }
    }
    return success; 
  }

  bool Loader::getNextPage(){
    bool success=false;
    Json::Reader reader;
    Json::Value root;
    string body;
    documents_.clear();
    index_=0;
    string path=kc::strprintf("/document/%s/?cursor=%s&limit=10",source_.c_str(),cursor_.c_str());
    if (getUrl(path,body)){
      if (reader.parse(body,root)){
        string next=root["cursors"]["next"].asString();
        success=next.size()>0;
        cursor_=next;
        const Json::Value rows = root["rows"];
        for (uint32_t i = 0;i<rows.size();++i){
          documents_.push_back(rows[i]);
        }
      }
    }
    return success;
  }
    
  bool Loader::getNextDocument(string& payload,uint32_t& doctype,uint32_t& docid){
    if(index_<documents_.size()){
      doctype=documents_[index_]["doctype"].asUInt();
      docid=documents_[index_]["docid"].asUInt();
      string path=kc::strprintf("/text/%d/%d/",doctype,docid);
      map<string,string> encoded;
      if(getUrl(path,encoded["text"])){
        vector<string> members=documents_[index_].getMemberNames();
        for (vector<string>::const_iterator it=members.begin(),ite=members.end();it!=ite;++it){
          if (*it!="id"){
            Json::Value value=documents_[index_][*it];
            string trimmed=value.toStyledString().substr(0,value.toStyledString().size()-1);
            if (isNumeric(trimmed)){
              encoded[*it]=trimmed;            
            }else{
              encoded[*it]=value.asString();
            }            
          }
        }
        kt::maptowwwform(encoded,&payload);
        index_++;  
        return true;
      }
    }
    return false;
  }
}