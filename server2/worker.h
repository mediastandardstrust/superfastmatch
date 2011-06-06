#ifndef _SFMWORKER_H                       // duplication check
#define _SFMWORKER_H

#include <map>
#include <sstream>
#include <kcutil.h>
#include <kthttp.h>
#include <kttimeddb.h>
#include <logger.h>
#include <document.h>

using namespace std;
using namespace kyototycoon;

namespace superfastmatch{
	class Worker : public HTTPServer::Worker {
	private:
		int32_t thnum_;
	  	std::map<std::string,TimedDB*> dbs_;	
		uint32_t windowsize_;
	public:	
		explicit Worker(int32_t thnum,const map<std::string,TimedDB*>& dbs,uint32_t windowsize){
			thnum_ = thnum;
			dbs_ = dbs;
			windowsize_ = windowsize;
		}
	private:
		struct RESTDirective{
			HTTPServer& serv;
			HTTPServer::Session& sess;
			HTTPClient::Method& verb;
			const string& path;
			const map<string, string>& reqheads;
			const string& reqbody;
			map<string, string>& resheads;
			string& resbody;
			string resource;
			string first_id;
			string second_id;
			string cursor;
			
			RESTDirective(HTTPServer& serv, 
						  HTTPServer::Session& sess,
	                  	  HTTPClient::Method& method,
						  const string& path,
						  const map<string, string>& reqheads,
						  const string& reqbody,
						  map<string, string>& resheads,
						  string& resbody,
						  const map<string, string>& misc
						):serv(serv),sess(sess),verb(method),path(path),reqheads(reqheads),reqbody(reqbody),resheads(resheads),resbody(resbody)
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

  		int32_t process(HTTPServer* serv, HTTPServer::Session* sess,
                  		const string& path, HTTPClient::Method method,
                  		const map<string, string>& reqheads,
                  		const string& reqbody,
                  		map<string, string>& resheads,
                  		string& resbody,
                  		const map<string, string>& misc) 
		{
			RESTDirective rest(*serv,*sess,method,path,reqheads,reqbody,resheads,resbody,misc);
			
			int32_t code;
			if (rest.resource=="echo"){
				code = echo(rest);
			}
			else if(rest.resource=="document"){
				code = document(rest);	
			}
			else{
				code = status(rest);
			}
			return code;
  		}

		int32_t document(RESTDirective& rest){
			stringstream message; 
			int32_t code;
			uint32_t doctype = kc::atoi(rest.first_id.data());
			uint32_t docid = kc::atoi(rest.second_id.data());
			Document document(doctype,docid,rest.reqbody.c_str(),windowsize_);
			switch(rest.verb){
				case HTTPClient::MGET:
			    case HTTPClient::MHEAD:
					if (document.load(dbs_["document"])){
						message << "Getting document: " << document;
						if(rest.verb==HTTPClient::MGET){
							rest.resbody.append(document.text());
						}
						rest.serv.log(Logger::INFO,message.str().c_str());	
						code=200;
					}else{
						message << "Error getting document: " << document;
						rest.serv.log(Logger::ERROR,message.str().c_str());	
						code=404;
					}
					break;					
				case HTTPClient::MPOST:
				case HTTPClient::MPUT:
					if (document.save(dbs_["document"])){ //Implement MapReduce on POST!!
						message << "Saving document: " << document;
						rest.serv.log(Logger::INFO,message.str().c_str());	
						code=200;
					}else{
						message << "Error saving document << document";
						rest.serv.log(Logger::ERROR,message.str().c_str());	
						code=500;
					}
					break;
				case HTTPClient::MDELETE:
					if (document.remove(dbs_["document"])){ 
						message << "Deleting document: " << document;
						rest.serv.log(Logger::INFO,message.str().c_str());	
						code=204;
					}else{
						message << "Error deleting document << document";
						rest.serv.log(Logger::ERROR,message.str().c_str());	
						code=404;
					}
					break;
				default:
					message << "Unknown command on: " << document;
					rest.serv.log(Logger::ERROR,message.str().c_str());
					code=500;
					break;
			}
			return code;
		}
		
		int32_t echo(RESTDirective& rest){
	      	for (map<string, string>::const_iterator it = rest.reqheads.begin();it != rest.reqheads.end(); it++) {
	        	if (!it->first.empty()) rest.resbody.append(it->first + ": ");
	        	rest.resbody.append(it->second + "\n");
	      	}
	      	rest.resbody.append(rest.reqbody);
	      	return 200;
		}
		
		int32_t status(RESTDirective& rest){
	      	rest.resbody.append("<h1>Status</h1");
	      	return 200;
		}

	};
}

#endif