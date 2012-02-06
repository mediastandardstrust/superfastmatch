#include <gflags/gflags.h>
#include <kthttp.h>
#include <logger.h>
#include <worker.h>
#include <registry.h>


using namespace superfastmatch;

DEFINE_bool(daemonize,false,"Run process in the background");

DEFINE_string(log_path,"-","Where to put the log files");

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
  // Registry registry;
  FlagsRegistry* registry = new FlagsRegistry();

  // prepare the worker
  Worker worker(registry);

  // prepare the server
  HTTPServer serv;
  stringstream network;
  network << registry->getAddress() << ":" <<registry->getPort();
  serv.set_network(network.str(), registry->getTimeout());
  serv.set_worker(&worker, registry->getThreadCount());

  // set up the logger
  Logger logger;
  logger.open(FLAGS_log_path.c_str());
  serv.set_logger(&logger, Logger::INFO | Logger::SYSTEM | Logger::ERROR);
  
  // Daemonize
  if (FLAGS_daemonize && !daemonize()){
    serv.log(Logger::SYSTEM, "Failed to daemonize!");
    return 1;
  }
  serv.log(Logger::SYSTEM, "================ [START]: pid=%d", getpid());

  g_serv = &serv;

  // start the server and block until it stops
  serv.start();
  
  // clean up connections and other resources
  serv.finish();

  delete registry;

  serv.log(Logger::SYSTEM, "================ [FINISH]: pid=%d", getpid());
  return 0;
}
