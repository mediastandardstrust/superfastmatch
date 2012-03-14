#ifndef _SFMAPI_H                       // duplication check
#define _SFMAPI_H

#include <re2/filtered_re2.h>
#include <kthttp.h>
#include <registry.h>
#include <worker.h>

namespace superfastmatch{
  // Forward Declarations
  class Api;
  struct ApiParams;
  struct ApiResponse;
  struct Matcher;
  
  // Typedefs
  typedef pair<int,string> response_t;
  typedef shared_ptr<Matcher> MatcherPtr;
  typedef shared_ptr<re2::RE2> RE2Ptr;
  typedef map<response_t,string> response_map;
  typedef pair<string,string> capture_t;
  typedef map<string,capture_t> capture_map;
  typedef void(Api::*ApiMethod)(const ApiParams& params,ApiResponse& response);
  
  // Classes
  struct ApiCall{
    const HTTPClient::Method verb;
    const string match;
    const response_map responses;
    const string description;
    const ApiMethod method;
    
    ApiCall(const HTTPClient::Method verb,
            const char* match,
            const response_map& responses,
            const char* description,
            const ApiMethod method);
  };
  
  struct ApiParams{
    map<string,string> resource;
    map<string,string> query;
    map<string,string> form;
    string body;
    //TODO: Remove this
    string url;    

    ApiParams(const HTTPClient::Method verb,const string& body, const map<string,string>& misc);
  };
  
  struct ApiResponse{
    response_t type;
    TemplateDictionary dict;
    
    ApiResponse();
  };
  
  struct Matcher {
    vector<string> atoms;
    vector<int> atom_indices;
    map<int,const ApiCall*> calls;
    map<int,RE2Ptr> regexes;
    re2::RE2::Options options;
    re2::FilteredRE2 f;
  };

  class Api{
  private:
    const static size_t METHOD_COUNT; 
    const static size_t API_COUNT; 
    const static capture_map captures_;
    const static ApiCall calls_[]; 

    
    Registry* registry_;
    vector<MatcherPtr> matchers_;;
    
  public:
    Api(Registry* registry);
    
    // Returns HTTP Status code or -1 if no match
    int Invoke(const string& path, HTTPClient::Method verb,
               const map<string, string>& reqheads,
               const string& reqbody,
               map<string, string>& resheads,
               string& resbody,
               const map<string, string>& misc);
    
  private:
    int MatchApiCall(MatcherPtr matcher,const string& path,ApiParams& params);
    
    void DoSearch(const ApiParams& params,ApiResponse& response);
    void GetDocument(const ApiParams& params,ApiResponse& response);
    void GetDocuments(const ApiParams& params,ApiResponse& response);
    void CreateDocument(const ApiParams& params,ApiResponse& response); 
    void CreateAndAssociateDocument(const ApiParams& params,ApiResponse& response);
    void DeleteDocument(const ApiParams& params,ApiResponse& response);
    void AssociateDocument(const ApiParams& params,ApiResponse& response);
    void AssociateDocuments(const ApiParams& params,ApiResponse& response);
    void GetIndex(const ApiParams& params,ApiResponse& response);
    void GetQueue(const ApiParams& params,ApiResponse& response);
    void GetPerformance(const ApiParams& params,ApiResponse& response);      
    void GetStatus(const ApiParams& params,ApiResponse& response);
    void GetHistogram(const ApiParams& params,ApiResponse& response);
    void GetDescription(const ApiParams& params,ApiResponse& response);
  };
}

#endif                                   // duplication check
