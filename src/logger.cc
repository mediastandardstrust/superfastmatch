#include "logger.h"

namespace superfastmatch{

  Logger::Logger() : strm_(NULL), lock_() {}

  Logger::~Logger() {
    if (strm_) close();
  }

  bool Logger::open(const char* path) {
    if (strm_) return false;
    if (path && *path != '\0' && std::strcmp(path, "-")) {
      std::ofstream* strm = new std::ofstream;
      strm->open(path, std::ios_base::out | std::ios_base::binary | std::ios_base::app);
      if (!*strm) {
        delete strm;
        return false;
      }
      strm_ = strm;
    } else {
      strm_ = &std::cout;
    }
    return true;
  }
  // close the stream
  void Logger::close() {
    if (!strm_) return;
    if (strm_ != &std::cout) delete strm_;
    strm_ = NULL;
  }

  void Logger::log(Kind kind, stringstream* s){
    log(kind,s->str().c_str());
    s->str("");
  }

  void Logger::log(Kind kind, const char* message) {
    if (!strm_) return;
    char date[48];
    kt::datestrwww(kc::nan(), kc::INT32MAX, 6, date);
    const char* kstr = "MISC";
    switch (kind) {
      case kt::HTTPServer::Logger::DEBUG: kstr = "DEBUG"; break;
      case kt::HTTPServer::Logger::INFO: kstr = "INFO"; break;
      case kt::HTTPServer::Logger::SYSTEM: kstr = "SYSTEM"; break;
      case kt::HTTPServer::Logger::ERROR: kstr = "ERROR"; break;
    }
    lock_.lock();
    *strm_ << date << ": [" << kstr << "]: " << message << "\n";
    strm_->flush();
    lock_.unlock();
  }	
}

