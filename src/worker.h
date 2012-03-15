#ifndef _SFMWORKER_H                       // duplication check
#define _SFMWORKER_H

#include <kthttp.h>
#include <kcthread.h>
#include <registry.h>
#include <logger.h>
#include <posting.h>
#include <queue.h>
#include <command.h>
#include <document.h>
#include <search.h>
#include <templates.h>
#include <api.h>

using namespace std;
using namespace kyototycoon;

namespace superfastmatch{
  class Worker : public HTTPServer::Worker {
  private:
    Registry* registry_;
    Api api_;
  public: 
    explicit Worker(Registry* registry);

    void process_idle(HTTPServer* serv);
    void process_timer(HTTPServer* serv);
    int32_t process(HTTPServer* serv, HTTPServer::Session* sess,
                    const string& path, HTTPClient::Method method,
                    const map<string, string>& reqheads,
                    const string& reqbody,
                    map<string, string>& resheads,
                    string& resbody,
                    const map<string, string>& misc);
  
  private:              
    int32_t process_static(const string& path, 
                           HTTPClient::Method method,
                           const map<string, string>& reqheads,
                           const string& reqbody,
                           map<string, string>& resheads,
                           string& resbody,
                           const map<string, string>& misc);
  };
}

#endif