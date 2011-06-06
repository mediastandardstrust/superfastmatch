#include <iostream>
#include <vector>
#include <map>
#include <kttimeddb.h>
#include <kthttp.h>
#include <string>
#include <logger.h>
#include <worker.h>

using namespace std;
using namespace kyototycoon;
using namespace superfastmatch;

// the flag whether the server is alive
HTTPServer* g_serv = NULL;

// stop the running server
static void stopserver(int signum) {
  if (g_serv) g_serv->stop();
  g_serv = NULL;
}

// main routine
int main(int argc, char** argv) {
	// need to parse these parameters
	int32_t thnum = 8;
	double timeout = 10;
	
  	// set the signal handler to stop the server
  	setkillsignalhandler(stopserver);

	// set up the DBS
	map<string,TimedDB*> dbs;
	dbs["document"] = new TimedDB();
	dbs["document"]->open("document.kct#bnum=100000#ktopts=p#zcomp=zlib");
	dbs["index"] = new TimedDB();
	dbs["index"]->open("index.kct#bnum=100000#ktopts=p");
	dbs["association"] = new TimedDB();
	dbs["association"]->open("association.kct#bnum=100000#ktopts=p");
	
	
  	// prepare the worker
  	Worker worker(thnum,dbs,15);

	// prepare the server
	HTTPServer serv;
	serv.set_network("127.0.0.1:1978", timeout);
	serv.set_worker(&worker, thnum);

	// set up the logger
	Logger logger;
	logger.open("-");
	serv.set_logger(&logger, Logger::INFO | Logger::SYSTEM | Logger::ERROR);
	serv.log(Logger::SYSTEM, "================ [START]: pid=%d", getpid());

	g_serv = &serv;

	// start the server and block until it stops
	serv.start();

	// clean up connections and other resources
	serv.finish();

	// close and delete dbs
	for (map<string,TimedDB*>::iterator it=dbs.begin();it!=dbs.end();it++){
		(*it).second->close();	
	}

	serv.log(Logger::SYSTEM, "================ [FINISH]: pid=%d", getpid());
	return 0;
}
