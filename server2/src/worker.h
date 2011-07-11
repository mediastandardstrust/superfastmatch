#ifndef _SFMWORKER_H                       // duplication check
#define _SFMWORKER_H

#include <map>
#include <sstream>
#include <cstdlib>
#include <google/malloc_extension.h>
#include <kthttp.h>
#include <kcthread.h>
#include <registry.h>
#include <logger.h>
#include <queue.h>
#include <document.h>
#include <queue.h>

using namespace std;
using namespace kyototycoon;

namespace superfastmatch{
	struct RESTRequest;
	struct RESTResponse;
	
	class Worker : public HTTPServer::Worker {
	private:
		Registry& registry_;
	public:	
		explicit Worker(Registry& registry):
		registry_(registry)
		{ }
		
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
		void process_index(const RESTRequest&, RESTResponse&);
		void process_queue(const RESTRequest&, RESTResponse&);
		void process_help(const RESTRequest&, RESTResponse&);
		void process_status(const RESTRequest&, RESTResponse&);
		void process_histograms(const RESTRequest&, RESTResponse&);
		void process_heap(const RESTRequest&, RESTResponse&);
		void process_echo(const RESTRequest&, RESTResponse&);
	};
}

#endif