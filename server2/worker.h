#ifndef _SFMWORKER_H                       // duplication check
#define _SFMWORKER_H

#include <map>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <kcutil.h>
#include <kthttp.h>
#include <kcthread.h>
#include <logger.h>
#include <document.h>
#include <index.h>
#include <registry.h>

using namespace std;
using namespace kyototycoon;

namespace superfastmatch{
	class Worker : public HTTPServer::Worker {
	private:
		Registry& registry_;
	public:	
		explicit Worker(Registry& registry):registry_(registry){
		}
	private:
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
			
			bool isNumeric(string& input){
				float f; 
				istringstream s(input); 
				return(s >> f);
			}
			
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
			int32_t code;
			stringstream message;
			
			RESTResponse(map<string,string>& resheads):
			resheads(resheads){}
		};

		void process_idle(HTTPServer* serv) {
			// serv->log(Logger::INFO,"Idle");
	    }
	    
	    void process_timer(HTTPServer* serv) {
			serv->log(Logger::INFO,"Processing command queue");
			kyotocabinet::Thread::sleep(5.0);
	    }

  		int32_t process(HTTPServer* serv, HTTPServer::Session* sess,
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
			else if(req.resource=="index"){
				process_index(req,res);
			}
			else if(req.resource=="echo"){
				process_echo(req,res);
			}
			else{
				process_status(req,res);
			}
		
			res.message << " Response Time: " << setiosflags(ios::fixed) << setprecision(4) << kyotocabinet::time()-start << " secs";
			if (res.code==500 || res.code==404){
				serv->log(Logger::ERROR,res.message.str().c_str());
			}else{
				serv->log(Logger::INFO,res.message.str().c_str());
			}
			resbody.append(res.body.str());
			return res.code;
  		}

		void process_document(const RESTRequest& req,RESTResponse& res){
			if (req.first_is_numeric && req.second_is_numeric){
				uint32_t doctype = kc::atoi(req.first_id.data());
				uint32_t docid = kc::atoi(req.second_id.data());
				Document doc(doctype,docid,req.reqbody.c_str(),registry_);
				Index index(registry_);
				bool updated=false;
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
					case HTTPClient::MPOST:
						if (doc.save(updated)){
							res.message << "Saved document: " << doc;
							if (updated){
								if (req.verb==HTTPClient::MPUT && index.create(doc)){
									res.message << " and updated index ";
									res.code=201;
								}else if(req.verb==HTTPClient::MPOST){
									res.message << " and deferred indexing";
									res.code=202;
								}
								else{
									res.message << " but failed to update index";
									res.code=500;
								}
							}else{
								res.message << " no change to index";
								res.code=200;
							}
						}else{
							res.message << "Error saving document: " << doc;
							res.code=500;
						}
						break;
					case HTTPClient::MDELETE:
						if (doc.remove()){ 
							res.message << "Deleting document: " << doc;
							res.code=204;
						}else{
							res.message << "Error deleting document << doc";
							res.code=404;
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
		
		void process_index(const RESTRequest& req,RESTResponse& res){
			Index index(registry_);
			switch(req.verb){
				case HTTPClient::MPOST:{
						Job job(registry_);
						if (!job.hasStarted() && !job.hasFinished()){
							//This should be on a new thread
							index.batch(job);
							res.body << "Completed job: "<< req.first_id <<endl;
							res.code=301;
							res.resheads["Location"]="/";
						}else{
							res.body << "That job has previously started or completed"<<endl;
							res.code=500;
						}
					}
					break;
				default:
					res.message << "Unknown command";
					res.code=500;
					break;
			}
		}
		
		void process_echo(const RESTRequest& req,RESTResponse& res){
	      	for (map<string, string>::const_iterator it = req.reqheads.begin();it != req.reqheads.end(); it++) {
	        	if (!it->first.empty()) res.body << it->first  << ": ";
				res.body << it->second << endl;;
	      	}
	      	res.body << req.reqbody;
	      	res.code=200;
		}
		
		void process_status(const RESTRequest& req,RESTResponse& res){
	      	res.body << "<h1>Status</h1";
			res.code=200;
		}
	};
}

#endif