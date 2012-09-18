#include "api.h"

namespace superfastmatch{
  RegisterTemplateFilename(SUCCESS_JSON, "JSON/success.tpl");
	RegisterTemplateFilename(QUEUED_JSON, "JSON/queued.tpl");
	RegisterTemplateFilename(FAILURE_JSON, "JSON/failure.tpl");
	RegisterTemplateFilename(DESCRIPTION_HTML, "HTML/description.tpl");
	RegisterTemplateFilename(TEXT, "TEXT/data.tpl");

  // -------------------  
  // QueryParameter members
  // -------------------

  QueryParameter::QueryParameter(const string& name,const string& description,const string& default_value,const string& validator):
  name_(name),
  description_(description),
  default_value_(default_value),  
  validator_(validator)
  {}
  
  void QueryParameter::validate(string& value){
    if (!RE2::FullMatch(value,validator_)){
      value=default_value_;
    }
  }
  
  void QueryParameter::fillDictionary(TemplateDictionary* dict){
    dict->SetValue("NAME",name_);
    dict->SetValue("DESCRIPTION",description_);
    dict->SetValue("DEFAULT_VALUE",default_value_);
  }

  // -------------------  
  // ApiResponse members
  // -------------------
  
  ApiResponse::ApiResponse():
  type(-1,"text/html"),
  dict("response")
  {}
    
  // -------------------  
  // ApiParams members
  // -------------------
    
  ApiParams::ApiParams(const HTTPClient::Method verb,const string& body)
  {
    if (verb==HTTPClient::MPOST || verb==HTTPClient::MPUT){
      this->body=body;
      wwwformtomap(body,&form);
    }
  }

  // -------------------
  // ApiCall members
  // -------------------
  
  ApiCall::ApiCall(const HTTPClient::Method verb,
                   const char* match,
                   const response_map& responses,
                   const set<string>& queries,
                   const char* description,
                   const ApiMethod method,
                   const bool autoDocId):
    verb(verb),
    match(match),
    responses(responses),
    queries(queries),
    description(description),
    method(method),
    autoDocId(autoDocId)
    {}
    
  // -------------------
  // Api members
  // -------------------
    
  const size_t Api::METHOD_COUNT=6;
    
  Api::Api(Registry* registry):
    registry_(registry)
  {
    for (size_t i=0;i<Api::METHOD_COUNT;i++){
      matchers_.push_back(MatcherPtr(new Matcher()));
    }
    for (size_t i=0;i<Api::API_COUNT;i++){
      MatcherPtr matcher=matchers_[calls_[i].verb];
      int atom_id;
      string match=calls_[i].match;
      for (capture_map::const_iterator it=captures_.begin(),ite=captures_.end();it!=ite;++it){
        Replace(match,it->first,it->second.first);
      }
      assert(matcher->f.Add(match,matcher->options,&atom_id)==re2::RE2::NoError);
      matcher->atom_indices.push_back(atom_id);
      matcher->regexes[atom_id]=RE2Ptr(new re2::RE2(match));
      assert(matcher->regexes[atom_id]->error_code()==0);
      matcher->calls[atom_id]=&calls_[i];
    }
    for(size_t i=0;i<METHOD_COUNT;i++){
      if (matchers_[i]->f.NumRegexps()>0){
        matchers_[i]->f.Compile(&matchers_[i]->atoms); 
      }
    }
  }
  
  int Api::Invoke(const string& path, HTTPClient::Method verb,
                   const map<string, string>& reqheads,
                   const string& reqbody,
                   map<string, string>& resheads,
                   string& resbody,
                   const map<string, string>& misc)
  {
    ApiParams params(verb,reqbody);
    ApiResponse response;
    string lowercase_path(path);
    kc::strtolower(&lowercase_path);
    MatcherPtr matcher=matchers_[verb];
    int id=MatchApiCall(matcher,lowercase_path,params);
    if(id!=-1){
      map<string,string>::const_iterator q=misc.find("query");
      MatchQuery(matcher->calls[id],q!=misc.end()?q->second:"",params);
      (this->*matcher->calls[id]->method)(params,response);
      map<response_t,string>::const_iterator r=matcher->calls[id]->responses.find(response.type);
      if (r!=matcher->calls[id]->responses.end()){
        registry_->getTemplateCache()->ExpandWithData(r->second,STRIP_BLANK_LINES,&response.dict,NULL,&resbody); 
      }
      resheads["content-type"] = response.type.second;
    }
    return response.type.first;
  } 
  
  int Api::MatchApiCall(MatcherPtr matcher,const string& path,ApiParams& params){
    int id=-1;
    if (matcher->f.NumRegexps()>0){
      id=matcher->f.FirstMatch(path,matcher->atom_indices);      
    }
    if (id!=-1){
      RE2Ptr regex=matcher->regexes[id];
      string* captures=new string[regex->NumberOfCapturingGroups()];
      RE2::Arg* argv=new RE2::Arg[regex->NumberOfCapturingGroups()];
      RE2::Arg** args=new RE2::Arg*[regex->NumberOfCapturingGroups()];
      for(int i=0;i<regex->NumberOfCapturingGroups();i++){
        argv[i]=&captures[i];
        args[i]=&argv[i];
      };
      if (re2::RE2::FullMatchN(path,*regex,args,regex->NumberOfCapturingGroups())){
        for (map<string,int>::const_iterator it=regex->NamedCapturingGroups().begin(),ite=regex->NamedCapturingGroups().end();it!=ite;++it){
          params.resource[it->first]=captures[it->second-1];
        }
      }
      if(matcher->calls[id]->autoDocId){
        uint32_t doctype=kc::atoi(params.resource.find("doctype")->second.c_str());
        assert(doctype!=0);
        params.resource["docid"]=registry_->getDocumentManager()->getDocId(doctype);
      }
      delete[] args;
      delete[] argv;
      delete[] captures;
    }
    return id;
  }

  void Api::MatchQuery(const ApiCall* call,const string& query,ApiParams& params){
    vector<string> queries,parts;
    map<string,string> temp;
    kc::strsplit(query,"&",&queries);
    for (vector<string>::const_iterator it=queries.begin();it!=queries.end();++it){
      kc::strsplit(*it,"=",&parts);
      if (parts.size()==2){
        size_t ksiz,vsiz;
        char* kbuf = kc::urldecode(parts[0].c_str(), &ksiz);
        char* vbuf = kc::urldecode(parts[1].c_str(), &vsiz);
        temp[kbuf]=vbuf;
        delete[] kbuf;
        delete[] vbuf;
      }
    }
    for(set<string>::const_iterator it=call->queries.begin(),ite=call->queries.end();it!=ite;++it){
      string value=temp[*it];
      queries_.find(*it)->second->validate(value);
      params.query[*it]=value;
    }
  }

  // --------------------
  // Api call definitions
  // --------------------

  const size_t Api::API_COUNT=22;

  const capture_map Api::captures_=create_map<string,capture_t >\
                                   ("<url>",capture_t("(?P<url>(?i)\\b((?:[a-z][\\w-]+:(?:/{1,3}|[a-z0-9%])|www\\d{0,3}[.]|[a-z0-9.\\-]+[.][a-z]{2,4}/)(?:[^\\s()<>]+|\\(([^\\s()<>]+|(\\([^\\s()<>]+\\)))*\\))+(?:\\(([^\\s()<>]+|(\\([^\\s()<>]+\\)))*\\)|[^\\s`!()\\[\\]{};:'\".,<>?«»“”‘’])))","A valid url"))\
                                   ("<doctype>",capture_t("(?P<doctype>\\d+)","A document type with a value between 1 and 4294967295"))\
                                   ("<docid>",capture_t("(?P<docid>\\d+)","A document id with a value between  1 and 4294967295"))\
                                   ("<source>",capture_t("(?P<source>(((\\d+-\\d+):?|(\\d+):?))+)","A range of doctypes. Eg. 1-2:5:7-9 which is equivalent to [1,2,5,7,8,9]"))\
                                   ("<target>",capture_t("(?P<target>(((\\d+-\\d+):?|(\\d+):?))+)","A range of doctypes. Eg. 1-2:5:7-9 which is equivalent to [1,2,5,7,8,9]"));
  const query_map Api::queries_=create_map<string,QueryParameterPtr>\
                                ("cursor",QueryParameterPtr(new QueryParameter("cursor","The document cursor consists of three parts separated by a colon.The first is the url-encoded value of the metadata value to start from. The second is the <doctype> and the third is the <docid>. These are needed in case multiple documents have the same metadata value. The <doctype> and <docid> can be omitted if not known and you will get the first match with respect to the metadata value.","","^[^:]+(:\\d+)?(:\\d+)?$")))\
                                ("start",QueryParameterPtr(new QueryParameter("start","The numeric id of the first result.","0","^\\d+$")))\
                                ("order_by",QueryParameterPtr(new QueryParameter("order_by","Url-encoded name of the metadata item to use for sorting. It can be prefixed with a minus sign to reverse the ordering.","doctype","^-?\\S+$")))\
                                ("limit",QueryParameterPtr(new QueryParameter("limit","The number of results to return in a single query.","100","^\\d+$")));

  const ApiCall Api::calls_[Api::API_COUNT]={
    ApiCall(HTTPClient::MPOST,
            "^/search/?$",
            create_map<response_t,string>(response_t(200,"application/json; charset=utf-8"),SUCCESS_JSON)\
                                         (response_t(400,"application/json; charset=utf-8"),FAILURE_JSON),
            set<string>(),
            "Search for text in all documents. There must be a form field with name text, otherwise returns response code 400.",
            &Api::DoSearch,
            false),
    ApiCall(HTTPClient::MPOST,
            "^/search/<target>/?$",
            create_map<response_t,string>(response_t(200,"application/json; charset=utf-8"),SUCCESS_JSON)\
                                         (response_t(400,"application/json; charset=utf-8"),FAILURE_JSON),  
            set<string>(),
            "Search for text in the specified target doctype range. There must be a form field with name text and <target> must be correctly formed, otherwise returns response code 400.",
            &Api::DoSearch,
            false),
    ApiCall(HTTPClient::MGET,
            "^/text/<doctype>/<docid>/?$",
            create_map<response_t,string>(response_t(200,"text/plain; charset=utf-8"),TEXT)\
                                         (response_t(400,"text/plain; charset=utf-8"),TEXT)\
                                         (response_t(404,"text/plain; charset=utf-8"),TEXT),
            set<string>(),
            "Get text of existing document. If document does not exist returns 404.",
            &Api::GetText,
            false),
   ApiCall(HTTPClient::MPOST,
           "^/load/<url>/<source>/?$",
           create_map<response_t,string>(response_t(202,"application/json; charset=utf-8"),QUEUED_JSON)\
                                        (response_t(400,"application/json; charset=utf-8"),FAILURE_JSON),
           set<string>(),
           "Load documents with specified source doctype range from the superfastmatch instance with the given url asynchronously. If <source> is badly formed returns response code 400.",
           &Api::LoadDocuments,
           false),
    ApiCall(HTTPClient::MGET,
            "^/document/?$",
            create_map<response_t,string>(response_t(200,"application/json; charset=utf-8"),SUCCESS_JSON),
            create_set<string>("cursor")("order_by")("limit"),
            "Get metadata and text of all documents.",
            &Api::GetDocuments,
            false),
    ApiCall(HTTPClient::MGET,
            "^/document/<source>/?$",
            create_map<response_t,string>(response_t(200,"application/json; charset=utf-8"),SUCCESS_JSON)\
                                         (response_t(400,"application/json; charset=utf-8"),FAILURE_JSON),
            create_set<string>("cursor")("order_by")("limit"),
            "Get metadata and text of all documents with specified doctype. If <source> is badly formed returns response code 400.",
            &Api::GetDocuments,
            false),
    ApiCall(HTTPClient::MGET,
            "^/document/<doctype>/<docid>/?$",
            create_map<response_t,string>(response_t(200,"application/json; charset=utf-8"),SUCCESS_JSON)\
                                         (response_t(400,"application/json; charset=utf-8"),FAILURE_JSON)\
                                         (response_t(404,"application/json; charset=utf-8"),FAILURE_JSON),
            set<string>(),
            "Get metadata and text of existing document. If document does not exist returns 404.",
            &Api::GetDocument,
            false),
    ApiCall(HTTPClient::MPOST,
           "^/document/<doctype>/?$",
           create_map<response_t,string>(response_t(202,"application/json; charset=utf-8"),QUEUED_JSON)\
                                        (response_t(400,"application/json; charset=utf-8"),FAILURE_JSON),
           set<string>(),
           "Create a new document asynchronously with an auto-assigned docid. There must be a form field with name text and a string of length greater than 0, otherwise returns response code 400.",
           &Api::CreateDocument,
           true),
    ApiCall(HTTPClient::MPUT,
           "^/document/<doctype>/?$",
           create_map<response_t,string>(response_t(202,"application/json; charset=utf-8"),QUEUED_JSON)\
                                        (response_t(400,"application/json; charset=utf-8"),FAILURE_JSON),
           set<string>(),
           "Create and associate a new document asynchronously with an auto-assigned docid. There must be a form field with name text and a string of length greater than 0, otherwise returns response code 400.",
           &Api::CreateAndAssociateDocument,
           true),
    ApiCall(HTTPClient::MPOST,
           "^/document/<doctype>/<docid>/?$",
           create_map<response_t,string>(response_t(202,"application/json; charset=utf-8"),QUEUED_JSON)\
                                        (response_t(400,"application/json; charset=utf-8"),FAILURE_JSON),
           set<string>(),
           "Create a new document asynchronously. There must be a form field with name text and a string of length greater than 0, otherwise returns response code 400.",
           &Api::CreateDocument,
           false),
    ApiCall(HTTPClient::MPUT,
           "^/document/<doctype>/<docid>/?$",
           create_map<response_t,string>(response_t(202,"application/json; charset=utf-8"),QUEUED_JSON)\
                                        (response_t(400,"application/json; charset=utf-8"),FAILURE_JSON),
           set<string>(),
           "Create and associate a new document asynchronously. There must be a form field with name text and a string of length greater than 0, otherwise returns response code 400.",
           &Api::CreateAndAssociateDocument,
           false),
    ApiCall(HTTPClient::MDELETE,
           "^/document/<doctype>/<docid>/?$",
           create_map<response_t,string>(response_t(202,"application/json; charset=utf-8"),QUEUED_JSON),
           set<string>(),
           "Delete a document asynchronously.",
           &Api::DeleteDocument,
           false),
    ApiCall(HTTPClient::MPOST,
          "^/association/<doctype>/<docid>/?$",
          create_map<response_t,string>(response_t(202,"application/json; charset=utf-8"),QUEUED_JSON),
          set<string>(),
          "Associate a document asynchronously. If document does not exist, queued item marked as failed",
          &Api::AssociateDocument,
          false),
    ApiCall(HTTPClient::MPOST,
          "^/association/<doctype>/<docid>/<target>/?$",
          create_map<response_t,string>(response_t(202,"application/json; charset=utf-8"),QUEUED_JSON)\
                                       (response_t(400,"application/json; charset=utf-8"),FAILURE_JSON),                                                                                                                                                                 
          set<string>(),
          "Associate a document asynchronously with a set of documents that match the specified target doc type range. If <target> is badly formed returns response code 400.",
          &Api::AssociateDocument,
          false),
    ApiCall(HTTPClient::MPOST,
          "^/associations/?$",
          create_map<response_t,string>(response_t(202,"application/json; charset=utf-8"),QUEUED_JSON),
          set<string>(),
          "Associate all documents asynchronously.",
          &Api::AssociateDocuments,
          false),   
    ApiCall(HTTPClient::MPOST,
          "^/associations/<source>/?$",
          create_map<response_t,string>(response_t(202,"application/json; charset=utf-8"),QUEUED_JSON)\
                                       (response_t(400,"application/json; charset=utf-8"),FAILURE_JSON),
          set<string>(),
          "Associate a set of documents asynchronously which match the specified source doc type range. If <source> is badly formed returns response code 400.",
          &Api::AssociateDocuments,
          false),
    ApiCall(HTTPClient::MPOST,
          "^/associations/<source>/<target>/?$",
          create_map<response_t,string>(response_t(202,"application/json; charset=utf-8"),QUEUED_JSON)\
                                       (response_t(400,"application/json; charset=utf-8"),FAILURE_JSON),
          set<string>(),
          "Associate a set of documents asynchronously which match the specified source doc type range with the target doc type range. If <source> or <target> are badly formed returns response code 400.",
          &Api::AssociateDocuments,
          false),
    ApiCall(HTTPClient::MGET,
           "^/index/?$",
           create_map<response_t,string>(response_t(200,"application/json; charset=utf-8"),SUCCESS_JSON),
           create_set<string>("start")("limit"),
           "Get the index data. Not fully implemented yet.",
           &Api::GetIndex,
           false),
    ApiCall(HTTPClient::MGET,
           "^/queue/?$",
           create_map<response_t,string>(response_t(200,"application/json; charset=utf-8"),SUCCESS_JSON),
           create_set<string>("start")("limit"),
           "Get the queue data. Not fully implemented yet.",
           &Api::GetQueue,
           false),
    ApiCall(HTTPClient::MGET,
           "^/performance/?$",
           create_map<response_t,string>(response_t(200,"application/json; charset=utf-8"),SUCCESS_JSON),
           set<string>(),
           "Get the performance data",
           &Api::GetPerformance,
           false),                                                                                                                                                                                                                                                                                 
    ApiCall(HTTPClient::MGET,
           "^/status/?$",
           create_map<response_t,string>(response_t(200,"application/json; charset=utf-8"),SUCCESS_JSON),
           set<string>(),
           "Get the status of the superfastmatch instance. Not fully implemented yet.",
           &Api::GetStatus,
           false),
    ApiCall(HTTPClient::MGET,
           "^/describe/?$",
           create_map<response_t,string>(response_t(200,"text/html"),DESCRIPTION_HTML),
           set<string>(),
           "Describe the superfastmatch API.",
           &Api::GetDescription,
           false)                                                 
  };

  // ------------------------
  // Api call implementations
  // ------------------------

  void Api::DoSearch(const ApiParams& params,ApiResponse& response){
    map<string,string>::const_iterator text=params.form.find("text");
    string t;
    if (params.resource.find("target")!=params.resource.end()){
      t=params.resource.find("target")->second;
    }
    DocumentQueryPtr target(new DocumentQuery(registry_,"",t));
    if (text==params.form.end()){
      response.type=response_t(400,"application/json; charset=utf-8");
      response.dict.SetValue("MESSAGE","No text field specified");    
    }  
    else if(!target->isValid()){
      response.type=response_t(400,"application/json; charset=utf-8");
      response.dict.SetValue("MESSAGE","Target is invalid");    
    }else{
      SearchPtr search=Search::createTemporarySearch(registry_,params.body,target);
      search->fillJSONDictionary(&response.dict,false);
      response.type=response_t(200,"application/json; charset=utf-8");
    }
  }

  void Api::GetText(const ApiParams& params,ApiResponse& response){
    uint32_t doctype = kc::atoi(params.resource.find("doctype")->second.c_str());
    uint32_t docid = kc::atoi(params.resource.find("docid")->second.c_str());
    if(doctype==0 || docid==0){
      response.type=response_t(400,"text/plain; charset=utf-8");   
      response.dict.SetValue("MESSAGE","Doc Type and Doc Id must be non-zero");     
    }else{
      DocumentPtr doc=registry_->getDocumentManager()->getDocument(doctype,docid,DocumentManager::TEXT);
      if (doc){
        response.dict.SetValue("DATA",doc->getText());
        response.type=response_t(200,"text/plain; charset=utf-8");
      }else{
        response.type=response_t(404,"text/plain; charset=utf-8");        
        response.dict.SetValue("MESSAGE","Document not found.");
      }
    }
  }

  void Api::GetDocument(const ApiParams& params,ApiResponse& response){
    uint32_t doctype = kc::atoi(params.resource.find("doctype")->second.c_str());
    uint32_t docid = kc::atoi(params.resource.find("docid")->second.c_str());
    if(doctype==0 || docid==0){
      response.type=response_t(400,"application/json; charset=utf-8");   
      response.dict.SetValue("MESSAGE","Doc Type and Doc Id must be non-zero");     
    }else{
      SearchPtr search=Search::getPermanentSearch(registry_,doctype,docid);
      if (search){
        search->fillJSONDictionary(&response.dict,true);
        response.type=response_t(200,"application/json; charset=utf-8");
      }else{
        response.type=response_t(404,"application/json; charset=utf-8");        
        response.dict.SetValue("MESSAGE","Document not found.");
      }
    }
  }

  void Api::LoadDocuments(const ApiParams& params,ApiResponse& response){
    string source=params.resource.find("source")->second;
    string url=params.resource.find("url")->second;
    DocumentQuery query(registry_,source,"",params.query);
    if (query.isValid()){
      CommandPtr loadCommand = registry_->getQueueManager()->createCommand(superfastmatch::LoadDocuments,0,0,source,"",url);
      loadCommand->fillDictionary(&response.dict);
      response.type=response_t(202,"application/json; charset=utf-8");
    }else{
      response.type=response_t(400,"application/json; charset=utf-8");        
      response.dict.SetValue("MESSAGE","Source is invalid.");
    }
  }
  
  void Api::GetDocuments(const ApiParams& params,ApiResponse& response){
    string source;
    if (params.resource.find("source")!=params.resource.end()){
      source=params.resource.find("source")->second;
    }
    DocumentQuery query(registry_,source,"",params.query);
    if (query.isValid()){
      query.fillJSONDictionary(&response.dict);
      response.type=response_t(200,"application/json; charset=utf-8");
    }else{
      response.type=response_t(400,"application/json; charset=utf-8");        
      response.dict.SetValue("MESSAGE","Source is invalid.");
    }
  }
  
  void Api::CreateDocument(const ApiParams& params,ApiResponse& response){
    uint32_t doctype=kc::atoi(params.resource.find("doctype")->second.c_str());
    uint32_t docid=kc::atoi(params.resource.find("docid")->second.c_str());      
    map<string,string>::const_iterator text=params.form.find("text");
    if(doctype==0 || docid==0){
      response.type=response_t(400,"application/json; charset=utf-8");   
      response.dict.SetValue("MESSAGE","Doc Type and Doc Id must be non-zero");     
    }else if (text!=params.form.end() && text->second.size()>0){
      CommandPtr addCommand = registry_->getQueueManager()->createCommand(AddDocument,doctype,docid,"","",params.body);
      addCommand->fillDictionary(&response.dict);
      response.type=response_t(202,"application/json; charset=utf-8");        
    }else{
      response.type=response_t(400,"application/json; charset=utf-8");   
      response.dict.SetValue("MESSAGE","No text field specified or is empty");     
    }
  }

  void Api::CreateAndAssociateDocument(const ApiParams& params,ApiResponse& response){
    CreateDocument(params,response);
    if (response.type.first==202){
      AssociateDocument(params,response);
    }
  }
  
  void Api::DeleteDocument(const ApiParams& params,ApiResponse& response){
    uint32_t doctype = kc::atoi(params.resource.find("doctype")->second.c_str());
    uint32_t docid = kc::atoi(params.resource.find("docid")->second.c_str());
    CommandPtr deleteCommand = registry_->getQueueManager()->createCommand(DropDocument,doctype,docid,"","","");
    deleteCommand->fillDictionary(&response.dict);
    response.type=response_t(202,"application/json; charset=utf-8");        
  }
  
  void Api::AssociateDocument(const ApiParams& params,ApiResponse& response){
    uint32_t doctype = kc::atoi(params.resource.find("doctype")->second.c_str());
    uint32_t docid = kc::atoi(params.resource.find("docid")->second.c_str());
    CommandPtr associateCommand = registry_->getQueueManager()->createCommand(AddAssociation,doctype,docid,"","","");
    associateCommand->fillDictionary(&response.dict);
    response.type=response_t(202,"application/json; charset=utf-8");
  }
  
  void Api::AssociateDocuments(const ApiParams& params,ApiResponse& response){
    string source;
    string target;
    if (params.resource.find("source")!=params.resource.end()){
      source=params.resource.find("source")->second;
    }
    if (params.resource.find("target")!=params.resource.end()){
      target=params.resource.find("target")->second;
    }
    DocumentQuery query(registry_,source,target,params.query);
    if (query.isValid()){
      CommandPtr associateCommand = registry_->getQueueManager()->createCommand(AddAssociations,0,0,source,target,"");
      associateCommand->fillDictionary(&response.dict);
      response.type=response_t(202,"application/json; charset=utf-8");      
    }else{
      response.type=response_t(400,"application/json; charset=utf-8");   
      response.dict.SetValue("MESSAGE","Source or target is badly formed.");
    }
  }
  
  void Api::GetIndex(const ApiParams& params,ApiResponse& response){
    uint64_t start = kc::atoi(params.query.find("start")->second.c_str());
    uint64_t limit = kc::atoi(params.query.find("limit")->second.c_str());
    registry_->getPostings()->fillListDictionary(&response.dict,start,limit);
    response.type=response_t(200,"application/json; charset=utf-8");
  }
  
  void Api::GetQueue(const ApiParams& params,ApiResponse& response){
    uint64_t start = kc::atoi(params.query.find("start")->second.c_str());
    uint64_t limit = kc::atoi(params.query.find("limit")->second.c_str());
    registry_->getQueueManager()->fillDictionary(&response.dict,start,limit);
    response.type=response_t(200,"application/json; charset=utf-8");    
  }
  
  void Api::GetPerformance(const ApiParams& params,ApiResponse& response){
    registry_->fillPerformanceDictionary(&response.dict);
    response.type=response_t(200,"application/json; charset=utf-8");    
  }
  
  void Api::GetStatus(const ApiParams& params,ApiResponse& response){
    registry_->fillStatusDictionary(&response.dict);
    registry_->getPostings()->fillStatusDictionary(&response.dict);
    response.type=response_t(200,"application/json; charset=utf-8");
  }
  
  void Api::GetHistogram(const ApiParams& params,ApiResponse& response){
    registry_->getPostings()->fillHistogramDictionary(&response.dict);
    response.type=response_t(200,"application/json; charset=utf-8");
  }
  
  void Api::GetDescription(const ApiParams& params,ApiResponse& response){
    for(size_t i=0;i<Api::API_COUNT;i++){
      HTTPClient::Method verb=calls_[i].verb;
      TemplateDictionary* resourceDict=response.dict.AddSectionDictionary("RESOURCE");
      resourceDict->SetValue("URL",calls_[i].match.substr(1,calls_[i].match.size()-3));
      resourceDict->SetValue("METHOD",verb==HTTPClient::MGET?"GET":verb==HTTPClient::MPOST?"POST":verb==HTTPClient::MPUT?"PUT":"DELETE");
      resourceDict->SetValue("DESCRIPTION",calls_[i].description);
      resourceDict->SetIntValue("RESPONSE_COUNT",calls_[i].responses.size());
      for (response_map::const_iterator it=calls_[i].responses.begin(),ite=calls_[i].responses.end();it!=ite;++it){
        TemplateDictionary* responseDict=resourceDict->AddSectionDictionary("RESPONSE");        
        responseDict->SetIntValue("CODE",it->first.first);
        responseDict->SetValue("CONTENT_TYPE",it->first.second);
      }
      for (set<string>::const_iterator it=calls_[i].queries.begin(),ite=calls_[i].queries.end();it!=ite;++it){
        TemplateDictionary* queryDict=resourceDict->AddSectionDictionary("QUERY");        
        queryDict->SetValue("ID",*it);
        queries_.find(*it)->second->fillDictionary(queryDict);
      }
    }
    for(capture_map::const_iterator it=captures_.begin(),ite=captures_.end();it!=ite;++it){
      TemplateDictionary* parameterDict=response.dict.AddSectionDictionary("PARAMETER");
      parameterDict->SetValue("TITLE",it->first);
      parameterDict->SetValue("DESCRIPTION",it->second.second);
    }
    for (map<string,QueryParameterPtr>::const_iterator it=queries_.begin(),ite=queries_.end();it!=ite;++it){
      TemplateDictionary* queryDict=response.dict.AddSectionDictionary("QUERIES");        
      queryDict->SetValue("ID",it->first);
      it->second->fillDictionary(queryDict);
    }
    vector<google::CommandLineFlagInfo> flags;
    google::GetAllFlags(&flags);
    const ptrdiff_t BUILTIN_FLAGS=14;
    for (vector<google::CommandLineFlagInfo>::const_iterator it=flags.begin()+BUILTIN_FLAGS,ite=flags.end();it!=ite;++it){
      TemplateDictionary* flagDict=response.dict.AddSectionDictionary("FLAGS");        
      flagDict->SetValue("NAME",it->name);
      flagDict->SetValue("TYPE",it->type);
      flagDict->SetValue("CURRENT_VALUE",it->current_value);
      flagDict->SetValue("DEFAULT_VALUE",it->default_value);
      flagDict->SetValue("DESCRIPTION",it->description);
    }
    response.type=response_t(200,"text/html");
  }
}
