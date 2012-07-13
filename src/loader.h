#ifndef _SFMLOADER_H                       // duplication check
#define _SFMLOADER_H

#include <common.h>
#include <logger.h>
#include <kthttp.h>
#include <json/json.h>

namespace superfastmatch{
  class Loader{
  private:
    kt::URL url_;
    kt::HTTPClient client_;
    map<string,string> headers_;
    string source_;
    string cursor_;
    uint32_t attempts_;
    bool open_;
    vector<Json::Value> documents_;
    size_t index_;
    Logger* logger_;
    
    bool getUrl(const string& path,string& body);
    
  public:
    Loader(Logger* logger,const string& url,const string& source);
    ~Loader();
    
    bool getNextPage();
    bool getNextDocument(string& payload,uint32_t& doctype,uint32_t& docid);
  };
}
#endif