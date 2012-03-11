#ifndef _SFMWORKER_H                       // duplication check
#define _SFMWORKER_H

// #include <google/malloc_extension.h>
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

using namespace std;
using namespace kyototycoon;

namespace superfastmatch{
  struct RESTRequest{
    const HTTPClient::Method& verb;
    const string& path;
    string url;
    const map<string, string>& reqheads;
    const string& reqbody;
    map<string,string> values;
    string resource;
    string first_id;
    string second_id;
    bool first_is_numeric;
    bool second_is_numeric;
    string cursor;
    bool cursor_is_numeric;
      
    RESTRequest(const HTTPClient::Method& method,
            const string& path,
            const map<string, string>& reqheads,
            const string& reqbody,
            const map<string, string>& misc);
  };
  
  struct RESTResponse{
      map<string, string>& resheads;
      TemplateDictionary dict;
      string template_name;
      int32_t code;
      string content_type;
      stringstream message;
      string& body;
    
      RESTResponse(map<string,string>& resheads,string& resbody);
  };
  
  class Worker : public HTTPServer::Worker {
  private:
    Registry* registry_;
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
    void process_search(const RESTRequest&, RESTResponse&);
    void process_document(const RESTRequest&, RESTResponse&);
    void process_association(const RESTRequest&, RESTResponse&);
    void process_index(const RESTRequest&, RESTResponse&);
    void process_queue(const RESTRequest&, RESTResponse&);
    void process_help(const RESTRequest&, RESTResponse&);
    void process_status(const RESTRequest&, RESTResponse&);
    void process_histograms(const RESTRequest&, RESTResponse&);
    void process_performance(const RESTRequest&, RESTResponse&);
    void process_heap(const RESTRequest&, RESTResponse&);
    void process_echo(const RESTRequest&, RESTResponse&);
    void process_static(const RESTRequest& ,RESTResponse&);
  };
}

#endif