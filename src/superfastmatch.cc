#include <gflags/gflags.h>
#include <kthttp.h>
#include <logger.h>
#include <worker.h>
#include <registry.h>
#include <signal.h>
#include <execinfo.h>

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

  // set usage message
  string usage("This program allows bulk text comparison.  Sample usage:\n");
  usage += argv[0];
  google::SetUsageMessage(usage);

  // parse command line options
  google::ParseCommandLineFlags(&argc, &argv, true);

  // set up the registry
  FlagsRegistry registry;

  // prepare the worker
  Worker worker(&registry);

  // prepare the server
  HTTPServer serv;
  stringstream network;
  network << registry.getAddress() << ":" <<registry.getPort();
  serv.set_network(network.str(), registry.getTimeout());
  serv.set_worker(&worker, registry.getThreadCount());

  // set up the logger
  serv.set_logger(registry.getLogger(), Logger::INFO | Logger::SYSTEM | Logger::ERROR);
  
  serv.log(Logger::SYSTEM, "================ [START]: pid=%d", getpid());
  g_serv = &serv;

  // start the server and block until it stops
  serv.start();

  // clean up connections and other resources
  registry.close();
  serv.finish();
  serv.log(Logger::SYSTEM, "================ [FINISH]: pid=%d", getpid());
  return 0;
}
