#include "worker.h"

namespace superfastmatch{
  // Register pages
	RegisterTemplateFilename(EMPTY_PAGE, "empty_page.tpl");
	RegisterTemplateFilename(ECHO_PAGE, "echo_page.tpl");
	RegisterTemplateFilename(ERROR_PAGE, "error_page.tpl");
	RegisterTemplateFilename(NOT_FOUND_PAGE, "not_found.tpl");
	RegisterTemplateFilename(SEARCH_PAGE, "search_page.tpl");
	RegisterTemplateFilename(QUEUE_PAGE, "queue_page.tpl");
	RegisterTemplateFilename(QUEUED_PAGE, "queued_page.tpl");
	RegisterTemplateFilename(RESULTS_PAGE, "results_page.tpl");
	RegisterTemplateFilename(STATUS_PAGE, "status_page.tpl");
	RegisterTemplateFilename(INDEX_PAGE, "index_page.tpl");
	RegisterTemplateFilename(DOCUMENTS_PAGE, "documents_page.tpl");
	RegisterTemplateFilename(DOCUMENT_PAGE, "document_page.tpl");
	RegisterTemplateFilename(HELP_PAGE, "help_page.tpl");
	RegisterTemplateFilename(HISTOGRAMS_PAGE, "histograms_page.tpl");
	RegisterTemplateFilename(PERFORMANCE_PAGE, "performance_page.tpl");
	RegisterTemplateFilename(SUCCESS_JSON, "JSON/success.tpl");
	RegisterTemplateFilename(FAILURE_JSON, "JSON/failure.tpl");
	
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
            const map<string, string>& misc):
    verb(method),
    path(path),
    reqheads(reqheads),
    reqbody(reqbody)
    {
      if (method==HTTPClient::MPOST|| method==HTTPClient::MPUT){
        wwwformtomap(reqbody,&values);
      }
      vector<string> sections,queries,parts;
      kc::strsplit(path, '/', &sections);
      if (sections.size()>1){
        resource = sections[1];
      }
      if (sections.size()>2){
        first_id = sections[2];
        first_is_numeric=isNumeric(first_id);
      }
      else{
        first_is_numeric=false;
      }
      if (sections.size()>3){
        second_id = sections[3];
        second_is_numeric = isNumeric(second_id);
      }
      else{
        second_is_numeric=false;
      }     
      for (map<string, string>::const_iterator it = misc.begin();it!=misc.end();it++){
        if (it->first=="query"){
          kc::strsplit(it->second,"&",&queries);
          for (vector<string>::const_iterator it=queries.begin();it!=queries.end();it++){
            kc::strsplit(*it,"=",&parts);
            if ((parts.size()==2) && (parts[0]=="cursor")){
              cursor=parts[1];
              cursor_is_numeric=isNumeric(cursor);
            }
          }       
        }else if(it->first=="url"){
          url=it->second;
        }
      }
    }
  };
  
  class RESTResponse{
  public:
    map<string, string>& resheads;
    TemplateDictionary dict;
    string template_name;
    int32_t code;
    string content_type;
    stringstream message;
    string& body;
    
    RESTResponse(map<string,string>& resheads,string& resbody):
    resheads(resheads),
    dict("response"),
    content_type("text/html"),
    body(resbody)
    {}
  };

  Worker::Worker(Registry* registry):
  registry_(registry)
  { }

  void Worker::process_idle(HTTPServer* serv) {
    // serv->log(Logger::INFO,"Idle");
  }
      
  void Worker::process_timer(HTTPServer* serv) {
    if(!registry_->getPostings()->isReady()){
      registry_->getPostings()->init();
    }
    else if (registry_->getQueueManager()->processQueue()>0){
      serv->log(Logger::INFO,"Finished processing command queue");
    };
  }

  int32_t Worker::process(HTTPServer* serv, HTTPServer::Session* sess,
                    const string& path, HTTPClient::Method method,
                    const map<string, string>& reqheads,
                    const string& reqbody,
                    map<string, string>& resheads,
                    string& resbody,
                    const map<string, string>& misc) 
  {
    double start = kyotocabinet::time();
    RESTRequest req(method,path,reqheads,reqbody,misc);
    RESTResponse res(resheads,resbody);
    
    if(req.resource=="document"){
      process_document(req,res);  
    }
    else if(req.resource=="association"){
      process_association(req,res);
    }
    else if(req.resource=="index"){
      process_index(req,res);
    }
    else if(req.resource=="queue"){
      process_queue(req,res);
    }
    else if(req.resource=="echo"){
      process_echo(req,res);
    }
    else if(req.resource=="heap"){
      process_heap(req,res);
    }
    else if(req.resource=="histograms"){
      process_histograms(req,res);
    }
    else if(req.resource=="performance"){
      process_performance(req,res);
    }
    else if(req.resource=="status"){
      process_status(req,res);
    }
    else if(req.resource=="help"){
      process_help(req,res);
    }
    else if(req.resource=="search"){
      process_search(req,res);
    }else{
      process_static(req,res);
    }
    registry_->getTemplateCache()->ExpandWithData(res.template_name,DO_NOT_STRIP,&res.dict,NULL,&resbody);

    stringstream time;
    time << setiosflags(ios::fixed) << setprecision(4) << kyotocabinet::time()-start << " secs";
    resheads["content-type"] = res.content_type;
    resheads["X-response-time"] = time.str();
    return res.code;
  }

  void Worker::process_search(const RESTRequest& req,RESTResponse& res){
    res.dict.SetTemplateGlobalValue("TITLE","Search");
    switch (req.verb){
      case HTTPClient::MPOST:{
          SearchPtr search=Search::createTemporarySearch(registry_,req.reqbody);
          search->fillJSONDictionary(&res.dict,false);
          res.content_type="application/json";
          res.code=200;
          res.template_name=SUCCESS_JSON;
        }
        break;
      case HTTPClient::MGET:
        res.template_name=SEARCH_PAGE;
        res.content_type="text/html";
        res.code=200;
        break;
      default:
        res.template_name=ERROR_PAGE;
        res.content_type="text/html";
        res.code=200;
        break;
    }
  }

  void Worker::process_document(const RESTRequest& req,RESTResponse& res){
    res.dict.SetTemplateGlobalValue("TITLE","Document");
    if (req.first_is_numeric && req.second_is_numeric){
      uint32_t doctype = kc::atoi(req.first_id.data());
      uint32_t docid = kc::atoi(req.second_id.data());
      switch(req.verb){
        case HTTPClient::MGET:
        case HTTPClient::MHEAD:
          {
            SearchPtr search=Search::getPermanentSearch(registry_,doctype,docid);
            if (search){
              search->fillJSONDictionary(&res.dict,true);
              res.template_name=SUCCESS_JSON; 
              res.code=200;
            }else{
              res.dict.SetValue("MESSAGE","Document not found");
              res.template_name=FAILURE_JSON;
              res.code=404;
            }
            res.content_type="application/json";
          }
          break;          
        case HTTPClient::MPUT:
        case HTTPClient::MPOST:{
            CommandPtr addCommand = registry_->getQueueManager()->createCommand(AddDocument,doctype,docid,req.reqbody);
            addCommand->fillDictionary(&res.dict);
            if (req.verb==HTTPClient::MPUT){
              CommandPtr associateCommand = registry_->getQueueManager()->createCommand(AddAssociation,doctype,docid,"");
              associateCommand->fillDictionary(&res.dict);
            }
            res.message << "Queued document: (" << doctype << "," << docid << ") for indexing";
            res.template_name=QUEUED_PAGE;
            res.dict.SetIntValue("QUEUE_ID",addCommand->getQueueId());
            res.dict.SetIntValue("DOC_TYPE",doctype);
            res.dict.SetIntValue("DOC_ID",docid);
            res.code=202;
          }
          break;
        case HTTPClient::MDELETE:{
            CommandPtr deleteCommand = registry_->getQueueManager()->createCommand(DropDocument,doctype,docid,"");
            deleteCommand->fillDictionary(&res.dict);
            res.message << "Queued document: (" << doctype << "," << docid << ") for deleting";
            res.code=202;
          }
          break;
        default:
          res.template_name=ERROR_PAGE;
          res.code=500;
          break;
      }
    }
    else{
      DocumentQuery query(registry_,req.url);
      if (query.isValid()){
        query.fillJSONDictionary(&res.dict);
        res.content_type="application/json";
        res.template_name=SUCCESS_JSON;
        res.code=200;
      }else{
        res.content_type="application/json";
        res.template_name=FAILURE_JSON;
        res.code=500;
      }
    }
  }
  
  void Worker::process_help(const RESTRequest& req,RESTResponse& res){
    res.dict.SetTemplateGlobalValue("TITLE","Help");
    res.template_name=HELP_PAGE;
    res.code=200;
  }
  
  void Worker::process_association(const RESTRequest& req, RESTResponse& res){
    res.dict.SetTemplateGlobalValue("TITLE","Association");
    uint32_t doctype=0;
    uint32_t docid=0;
    if (req.first_is_numeric){
      doctype = kc::atoi(req.first_id.data());
    }
    if(req.second_is_numeric){
      docid = kc::atoi(req.second_id.data());
    }
    switch (req.verb){
      case HTTPClient::MPOST:{
        CommandPtr associateCommand = registry_->getQueueManager()->createCommand(AddAssociations,doctype,docid,"");
        associateCommand->fillDictionary(&res.dict);
        res.message << "Queued Association Task";
        res.template_name=QUEUED_PAGE;
        res.code=202;
        break;
      }
      default:
        res.code=500;
        break;
    }
  }
    
  void Worker::process_index(const RESTRequest& req,RESTResponse& res){
    res.dict.SetTemplateGlobalValue("TITLE","Index");
    switch(req.verb){
      case HTTPClient::MGET:
        if (req.cursor_is_numeric){
          registry_->getPostings()->fill_list_dictionary(&res.dict,kc::atoi(req.cursor.c_str()));
        }else{
          registry_->getPostings()->fill_list_dictionary(&res.dict,0);
        }
        res.template_name=INDEX_PAGE;
        res.code=200;
        break;
      default:
        res.template_name=ERROR_PAGE;
        res.code=500;
        break;
    }
  }
  
  void Worker::process_queue(const RESTRequest& req,RESTResponse& res){
    res.dict.SetTemplateGlobalValue("TITLE","Queue");
    switch (req.verb){
      case HTTPClient::MGET:
        if (req.cursor_is_numeric){
          registry_->getQueueManager()->fillDictionary(&res.dict,kc::atoi(req.cursor.c_str()));
        }else{
          registry_->getQueueManager()->fillDictionary(&res.dict);
        }
        res.template_name=QUEUE_PAGE;
        res.code=200;
        break;
      default:
        res.template_name=ERROR_PAGE;
        res.code=500;
        break;
    }
  }
  
  void Worker::process_heap(const RESTRequest& req, RESTResponse& res){
    // MallocExtensionWriter out;
    // MallocExtension::instance()->GetHeapSample(&out);
    // res.dict.SetValue("BODY",out);
    res.template_name=EMPTY_PAGE;
    res.code=200;
  }
  
  void Worker::process_echo(const RESTRequest& req,RESTResponse& res){
    for (map<string, string>::const_iterator it = req.reqheads.begin();it != req.reqheads.end(); it++) {
      if (!it->first.empty()){
        TemplateDictionary* header_dict=res.dict.AddSectionDictionary("HEADER");
        header_dict->SetValue("KEY",it->first);
        header_dict->SetValue("VALUE",it->second); 
      }
    }
    res.dict.SetValue("BODY",req.reqbody);
    res.template_name=ECHO_PAGE;
    res.code=200;
  }
  
  void Worker::process_histograms(const RESTRequest& req,RESTResponse& res){
    res.dict.SetTemplateGlobalValue("TITLE","Histograms");
    registry_->getPostings()->fill_histogram_dictionary(&res.dict);
    res.template_name=HISTOGRAMS_PAGE;
    res.code=200;
  }
  
  void Worker::process_performance(const RESTRequest& req,RESTResponse& res){
    res.dict.SetTemplateGlobalValue("TITLE","Performance");
    registry_->fillPerformanceDictionary(&res.dict);
    res.template_name=PERFORMANCE_PAGE;
    res.code=200;
  }
  
  void Worker::process_status(const RESTRequest& req,RESTResponse& res){
    size_t memory;
    const int kBufferSize = 16 << 12;
    char* buffer = new char[kBufferSize];
    // MallocExtension::instance()->GetNumericProperty("generic.current_allocated_bytes",&memory);
    // MallocExtension::instance()->GetStats(buffer,kBufferSize);
    res.dict.SetFormattedValue("MEMORY","%.4f",double(memory)/1024/1024/1024);
    res.dict.SetValue("MEMORY_STATS",string(buffer));
    res.dict.SetIntValue("WHITESPACE_HASH",registry_->getWhiteSpaceHash());
    delete [] buffer;
    registry_->fill_status_dictionary(&res.dict);
    registry_->getPostings()->fill_status_dictionary(&res.dict);
    res.dict.SetTemplateGlobalValue("TITLE","Status");
    res.template_name=STATUS_PAGE;
    res.code=200;
  }
  
  static const char* media_type(const std::string& url) {
    static const char* types[] = {
      "txt", "text/plain", "text", "text/plain", "asc", "text/plain",
      "js", "application/javascript","css","text/css",
      "html", "text/html", "htm", "text/html",
      "xml", "application/xml",
      "xhtml", "application/xml+xhtml",
      "gz", "application/x-gzip",
      "bz2", "application/x-bzip2",
      "zip", "application/zip",
      "pdf", "application/pdf",
      "png", "image/png",
      "jpg", "image/jpeg", "jpeg", "image/jpeg",
      "gif", "image/gif",
      NULL
    };
    const char* rp = url.c_str();
    const char* pv = std::strrchr(rp, '/');
    if (pv) rp = pv + 1;
    pv = std::strrchr(rp, '.');
    if (pv) {
      rp = pv + 1;
      for (int32_t i = 0; types[i]; i += 2) {
        if (!kc::stricmp(rp, types[i])) return types[i+1];
      }
    }
    return NULL;
  }
  
  // Lifted from ktuilserv.cc in Kyoto Tycoon
  void Worker::process_static(const RESTRequest& req,RESTResponse& res){
    const std::string& lpath = kt::HTTPServer::localize_path(req.path);
    std::string apath = registry_->getPublicPath() + (lpath.empty() ? "" : kc::File::PATHSTR) + lpath;
    bool dir = kc::strbwm(req.url.c_str(), "/");
    if (req.verb == kt::HTTPClient::MGET) {
      kc::File::Status sbuf;
      if (kc::File::status(apath, &sbuf)) {
        if (dir && sbuf.isdir) {
          const std::string& ipath = apath + kc::File::PATHSTR + "index.html";
          if (kc::File::status(ipath,&sbuf)) {
            apath = ipath;
          }
        }
        if (sbuf.isdir) {
          res.code = 403;
        } else {
          map<string,string>::const_iterator etag=req.reqheads.find("if-none-match");
          if ((etag!=req.reqheads.end())&&(etag->second.compare(toString(sbuf.mtime))==0)){
            res.code=304;
          }else{
            int64_t size;
            char* buf = kc::File::read_file(apath, &size, 256LL << 20);
            if (buf) {
              res.code = 200;
              res.resheads["ETag"]=toString(sbuf.mtime);
              const char* type = media_type(apath);
              if (type) res.content_type = type;
              res.body.append(buf, size);
              delete[] buf;
            } else {
              res.code = 403;
            } 
          }
        }
      } else {
        res.code = 404;
      }
    } else {
      res.code = 403;
    }
    if (res.code!=200){
      res.content_type = "text/plain";
      kc::strprintf(&res.body, "%s\n", kt::HTTPServer::status_name(res.code));
    }
  }
}
