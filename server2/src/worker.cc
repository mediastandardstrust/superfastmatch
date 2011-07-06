#include "worker.h"

namespace superfastmatch{
	struct RESTRequest{
		const HTTPClient::Method& verb;
		const string& path;
		const map<string, string>& reqheads;
		const string& reqbody;
		string resource;
		string first_id;
		string second_id;
		bool first_is_numeric;
		bool second_is_numeric;
		string cursor;
			
		RESTRequest(  const HTTPClient::Method& method,
					  const string& path,
					  const map<string, string>& reqheads,
					  const string& reqbody,
					  const map<string, string>& misc
					):verb(method),path(path),reqheads(reqheads),reqbody(reqbody)
		{
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
						}
					}				
				}
			}
		}
	};
	
	struct RESTResponse{
		map<string, string>& resheads;
		stringstream body;
		TemplateDictionary dict;
		string template_name;
		int32_t code;
		stringstream message;
		
		RESTResponse(map<string,string>& resheads):
		resheads(resheads),dict("response"){}
	};

	void Worker::process_idle(HTTPServer* serv) {
		// serv->log(Logger::INFO,"Idle");
	}
	    
    void Worker::process_timer(HTTPServer* serv) {
		Queue queue(registry_);
		if(!registry_.postings->isReady()){
			registry_.postings->init();
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
		registry_.templates->ReloadAllIfChanged(TemplateCache::LAZY_RELOAD);
		RESTRequest req(method,path,reqheads,reqbody,misc);
		RESTResponse res(resheads);
		
		if(req.resource=="document"){
			process_document(req,res);	
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
		else{
			process_status(req,res);
		}
		
		TemplateDictionary* header_dict = res.dict.AddIncludeDictionary("HEADER");
		header_dict->SetFilename(HEADER);
		TemplateDictionary* footer_dict = res.dict.AddIncludeDictionary("FOOTER");
		footer_dict->SetFilename(FOOTER);
		// res.dict.Dump();
		ExpandTemplate(res.template_name,DO_NOT_STRIP,&res.dict,&resbody);
	
		res.message << " Response Time: " << setiosflags(ios::fixed) << setprecision(4) << kyotocabinet::time()-start << " secs";
		if (res.code==500 || res.code==404){
			serv->log(Logger::ERROR,res.message.str().c_str());
		}else{
			serv->log(Logger::INFO,res.message.str().c_str());
		}
		// resbody.append(res.body.str());
		return res.code;
 	}

	void Worker::process_document(const RESTRequest& req,RESTResponse& res){
		if (req.first_is_numeric && req.second_is_numeric){
			uint32_t doctype = kc::atoi(req.first_id.data());
			uint32_t docid = kc::atoi(req.second_id.data());
			Document doc(doctype,docid,req.reqbody.c_str(),registry_);
			Queue queue(registry_);
			switch(req.verb){
				case HTTPClient::MGET:
			    case HTTPClient::MHEAD:
					if (doc.load()){
						res.message << "Getting document: " << doc;
						if(req.verb==HTTPClient::MGET){
							doc.serialize(res.body);
						}
						res.code=200;
					}else{
						res.message << "Error getting document: " << doc;
						res.code=404;
					}
					break;					
				case HTTPClient::MPUT:
				case HTTPClient::MPOST:{
						uint64_t queue_id = queue.add_document(doctype,docid,req.reqbody,req.verb==HTTPClient::MPUT);
						res.message << "Queued document: " <<  queue_id << " for indexing queue id:"<< queue_id;
						res.body << queue_id;
						res.code=202;
					}
					break;
				case HTTPClient::MDELETE:{
						uint64_t queue_id = queue.delete_document(doctype,docid);
						res.message << "Queued document: " << doc << " for deleting with queue id:" << queue_id;
						res.body << queue_id;
						res.code=202;
					}
					break;
				default:
					res.message << "Unknown command on: " << doc;
					res.code=500;
					break;
			}
		}
		else if (req.first_is_numeric){
			// Do doctype stuff
			res.code=200;
		}
		else{
			//Do document index stuff
			res.code=200;
		}
	}
		
	void Worker::process_index(const RESTRequest& req,RESTResponse& res){
		res.dict.SetTemplateGlobalValue("TITLE","Index");
		switch(req.verb){
			case HTTPClient::MGET:
				registry_.postings->fill_list_dictionary(&res.dict,0);
				res.template_name=INDEX_PAGE;
				res.code=200;
				break;
			default:
				res.message << "Unknown command";
				res.code=500;
				break;
		}
	}
	
	void Worker::process_queue(const RESTRequest& req,RESTResponse& res){
		Queue queue(registry_);
		switch (req.verb){
			default:
				queue.toString(res.body);
				res.code=200;
				break;
		}
	}
	
	void Worker::process_heap(const RESTRequest& req, RESTResponse& res){
		MallocExtensionWriter out;
		MallocExtension::instance()->GetHeapSample(&out);
		res.code=200;
		res.body << out;
	}
	
	void Worker::process_echo(const RESTRequest& req,RESTResponse& res){
      	for (map<string, string>::const_iterator it = req.reqheads.begin();it != req.reqheads.end(); it++) {
        	if (!it->first.empty()) res.body << it->first  << ": ";
			res.body << it->second << endl;;
      	}
      	res.body << req.reqbody;
      	res.code=200;
	}
	
	// (TODO) This is ugly!
	void Worker::process_status(const RESTRequest& req,RESTResponse& res){
		size_t memory;
		const int kBufferSize = 16 << 12;
		char* buffer = new char[kBufferSize];
		MallocExtension::instance()->GetNumericProperty("generic.current_allocated_bytes",&memory);
		MallocExtension::instance()->GetStats(buffer,kBufferSize);
		res.dict.SetFormattedValue("MEMORY","%.4f",double(memory)/1024/1024/1024);
		res.dict.SetValue("MEMORY_STATS",string(buffer));
		registry_.fill_status_dictionary(&res.dict);
		delete [] buffer;
		TemplateDictionary* postings_dict=res.dict.AddIncludeDictionary("POSTING_STATS");
		res.dict.SetTemplateGlobalValue("TITLE","Status");
		postings_dict->SetFilename(POSTING_STATS);
		registry_.postings->fill_histogram_dictionary(postings_dict);
		res.template_name=STATUS_PAGE;
		res.code=200;
	}
}