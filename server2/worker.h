#ifndef _SFMWORKER_H                       // duplication check
#define _SFMWORKER_H

#include <map>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <kcutil.h>
#include <kthttp.h>
#include <kttimeddb.h>
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
				}
				if (sections.size()>3){
					second_id = sections[3];
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
				document(req,res);	
			}
			else if(req.resource=="index"){
				index(req,res);
			}
			else if(req.resource=="echo"){
				echo(req,res);
			}
			else{
				status(req,res);
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

		void document(const RESTRequest& req,RESTResponse& res){
			uint32_t doctype = kc::atoi(req.first_id.data());
			uint32_t docid = kc::atoi(req.second_id.data());
			Document document(doctype,docid,req.reqbody.c_str(),registry_);
			Index index(registry_);
			switch(req.verb){
				case HTTPClient::MGET:
			    case HTTPClient::MHEAD:
					if (document.load()){
						res.message << "Getting document: " << document;
						if(req.verb==HTTPClient::MGET){
							document.serialize(res.body);
						}
						res.code=200;
					}else{
						res.message << "Error getting document: " << document;
						res.code=404;
					}
					break;					
				case HTTPClient::MPUT:
					if (document.save()){
						if(index.create(document)){
							res.message << "Saved and Added document: " << document << " to index";
							res.code=200;
						}
						else{
							res.message << "Error adding document: "<< document <<" to index";
							res.code=500;
						}
					}else{
						res.message << "Error saving document: " << document;
						res.code=500;
					}
					break;
				case HTTPClient::MPOST:
					if (document.save()){
						if(index.batch(document)){
							res.message << "Saved and Added document: " << document << " to index";
							res.code=200;
						}
						else{
							res.message << "Error adding document: "<< document <<" to index";
							res.code=500;
						}
						res.code=200;
					}else{
						res.message << "Error saving document << document";
						res.code=500;
					}
					break;
				case HTTPClient::MDELETE:
					if (document.remove()){ 
						res.message << "Deleting document: " << document;
						res.code=204;
					}else{
						res.message << "Error deleting document << document";
						res.code=404;
					}
					break;
				default:
					res.message << "Unknown command on: " << document;
					res.code=500;
					break;
			}
		}
		
		void index(const RESTRequest& req,RESTResponse& res){
			switch(req.verb){
				case HTTPClient::MPOST:
					res.code=200;
					break;
				default:
					res.message << "Unknown command";
					res.code=500;
					break;
			}
		}
		
		void echo(const RESTRequest& req,RESTResponse& res){
	      	for (map<string, string>::const_iterator it = req.reqheads.begin();it != req.reqheads.end(); it++) {
	        	if (!it->first.empty()) res.body << it->first  << ": ";
				res.body << it->second << endl;;
	      	}
	      	res.body << req.reqbody;
	      	res.code=200;
		}
		
		void status(const RESTRequest& req,RESTResponse& res){
	      	res.body << "<h1>Status</h1";
			res.code=200;
		}

	};
}

#endif