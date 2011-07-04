#include <kthttp.h>
#include <logger.h>
#include <worker.h>
#include <registry.h>

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
  	// set the signal handler to stop the server
  	setkillsignalhandler(stopserver);

	// set up the registry
	Registry registry("superfastmatch.cfg");
	
  	// prepare the worker
  	Worker worker(registry);

	// prepare the server
	HTTPServer serv;
	serv.set_network("127.0.0.1:1978", registry.timeout);
	serv.set_worker(&worker, registry.thread_count);

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

	serv.log(Logger::SYSTEM, "================ [FINISH]: pid=%d", getpid());
	return 0;
}
