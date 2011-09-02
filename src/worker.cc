#include "worker.h"

namespace superfastmatch{
  struct RESTRequest{
    const HTTPClient::Method& verb;
    const string& path;
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
        }
      }
    }
  };
  
  struct RESTResponse{
    map<string, string>& resheads;
    TemplateDictionary dict;
    string template_name;
    int32_t code;
    stringstream message;
    
    RESTResponse(map<string,string>& resheads):
    resheads(resheads),
    dict("response"){}
  };

  Worker::Worker(Registry* registry):
  registry_(registry)
  { }

  void Worker::process_idle(HTTPServer* serv) {
    // serv->log(Logger::INFO,"Idle");
  }
      
    void Worker::process_timer(HTTPServer* serv) {
    Queue queue(registry_);
    if(!registry_->getPostings()->isReady()){
      registry_->getPostings()->init();
    }
    else if (queue.process()){
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
    RESTResponse res(resheads);
    
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
    else if(req.resource=="status"){
      process_status(req,res);
    }
    else if(req.resource=="help"){
      process_help(req,res);
    }
    else if(req.resource=="search"){
      process_search(req,res);
    }else{
      process_search(req,res);
    }
    
    res.dict.AddIncludeDictionary("HEADER")->SetFilename(HEADER);
    res.dict.AddIncludeDictionary("FOOTER")->SetFilename(FOOTER);
    registry_->getTemplateCache()->ExpandWithData(res.template_name,DO_NOT_STRIP,&res.dict,NULL,&resbody);
  
    res.message << " Response Time: " << setiosflags(ios::fixed) << setprecision(4) << kyotocabinet::time()-start << " secs";
    if (res.code==500 || res.code==404){
      serv->log(Logger::ERROR,res.message.str().c_str());
    }else{
      serv->log(Logger::INFO,res.message.str().c_str());
    }
    return res.code;
  }

  void Worker::process_search(const RESTRequest& req,RESTResponse& res){
    res.dict.SetTemplateGlobalValue("TITLE","Search");
    switch (req.verb){
      case HTTPClient::MPOST:{
          DocumentPtr doc=registry_->getDocumentManager()->createTemporaryDocument(req.reqbody);
          registry_->getPostings()->fill_search_dictionary(doc,&res.dict); 
          res.code=200;
          res.template_name=RESULTS_PAGE;
        }
        break;
      case HTTPClient::MGET:
        res.template_name=SEARCH_PAGE;
        res.code=200;
        break;
      default:
        res.template_name=ERROR_PAGE;
        res.code=500;
        break;
    }
  }

  void Worker::process_document(const RESTRequest& req,RESTResponse& res){
    res.dict.SetTemplateGlobalValue("TITLE","Document");
    if (req.first_is_numeric && req.second_is_numeric){
      uint32_t doctype = kc::atoi(req.first_id.data());
      uint32_t docid = kc::atoi(req.second_id.data());
      Queue queue(registry_);
      switch(req.verb){
        case HTTPClient::MGET:
        case HTTPClient::MHEAD:
          {
            DocumentPtr doc=registry_->getDocumentManager()->getDocument(doctype,docid);
            if (doc){
              res.message << "Getting document: " << doc;
              if(req.verb==HTTPClient::MGET){
                res.template_name=DOCUMENT_PAGE;
                doc->fill_document_dictionary(&res.dict);
              }
              res.code=200;
            }else{
              res.message << "Error getting document: " << doc;
              res.code=404;
            } 
          }
          break;          
        case HTTPClient::MPUT:
        case HTTPClient::MPOST:{
            uint64_t queue_id = queue.add_document(doctype,docid,req.reqbody,req.verb==HTTPClient::MPUT);
            res.message << "Queued document: (" << doctype << "," << docid << ") for indexing queue id:"<< queue_id;
            res.dict.SetIntValue("QUEUE_ID",queue_id);
            res.dict.SetIntValue("DOC_TYPE",doctype);
            res.dict.SetIntValue("DOC_ID",docid);
            res.dict.SetValue("ACTION","insertion into index");
            res.template_name=QUEUED_PAGE;
            res.code=202;
          }
          break;
        case HTTPClient::MDELETE:{
            uint64_t queue_id = queue.delete_document(doctype,docid);
            res.message << "Queued document: (" << doctype << "," << docid << ") for deleting with queue id:" << queue_id;
            res.dict.SetIntValue("QUEUE_ID",queue_id);
            res.dict.SetIntValue("DOC_TYPE",doctype);
            res.dict.SetIntValue("DOC_ID",docid);
            res.dict.SetValue("ACTION","deletion from index");
            res.code=202;
          }
          break;
        default:
          res.template_name=ERROR_PAGE;
          res.code=500;
          break;
      }
    }
    else if (req.first_is_numeric){
      DocumentCursor cursor(registry_);
      uint32_t doctype=kc::atoi(req.first_id.c_str());
      uint32_t docid=0;
      if (req.cursor_is_numeric){
        docid=kc::atoi(req.cursor.c_str());
      }
      cursor.fill_list_dictionary(&res.dict,doctype,docid);
      res.template_name=DOCUMENTS_PAGE;
      res.code=200;
    }
    else{
      DocumentCursor cursor(registry_);
      uint32_t docid=0;
      if (req.cursor_is_numeric){
        docid=kc::atoi(req.cursor.c_str());
      }
      cursor.fill_list_dictionary(&res.dict,0,docid);
      res.template_name=DOCUMENTS_PAGE;
      res.code=200;
    }
  }
  
  void Worker::process_help(const RESTRequest& req,RESTResponse& res){
    res.dict.SetTemplateGlobalValue("TITLE","Help");
    res.template_name=HELP_PAGE;
    res.code=200;
  }
  
  void Worker::process_association(const RESTRequest& req, RESTResponse& res){
    res.dict.SetTemplateGlobalValue("TITLE","Association");
    Queue queue(registry_);
    switch (req.verb){
      case HTTPClient::MPOST:{
        queue.addAssociations();
        res.code=200;
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
    Queue queue(registry_);
    res.dict.SetTemplateGlobalValue("TITLE","Queue");
    switch (req.verb){
      case HTTPClient::MGET:
        queue.fill_list_dictionary(&res.dict);
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
}
