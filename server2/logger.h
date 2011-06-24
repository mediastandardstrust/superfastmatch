#ifndef _SFMLOGGER_H                       // duplication check
#define _SFMLOGGER_H

#include <kthttp.h>
#include <iostream>

using namespace std;
using namespace kyototycoon;

namespace superfastmatch{

class DBLogger : public kc::BasicDB::Logger {
	 public:
	  // constructor
	  explicit DBLogger() : strm_(NULL) {}
	  // destructor
	  ~DBLogger() {
	    if (strm_) close();
	  }
	  // open the stream
	  bool open(const char* path) {
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
	  void close() {
	    if (!strm_) return;
	    if (strm_ != &std::cout) delete strm_;
	    strm_ = NULL;
	  }
	  // process a log message.
	  void log(const char* file, int32_t line, const char* func, Kind kind,const char* message) {
	    if (!strm_) return;
	    char date[48];
	    datestrwww(kc::nan(), kc::INT32MAX, 6, date);
	    const char* kstr = "MISC";
	    switch (kind) {
	        case kc::BasicDB::Logger::DEBUG: kstr = "DEBUG"; break;
	        case kc::BasicDB::Logger::INFO: kstr = "INFO"; break;
	        case kc::BasicDB::Logger::WARN: kstr = "WARN"; break;
	        case kc::BasicDB::Logger::ERROR: kstr = "ERROR"; break;
		}
	    *strm_ << date << ":  [" << kstr << "]: " << file << ": " << line << ": " << func << ": " << message << std::endl;
	    strm_->flush();
	  }
	 private:
	  std::ostream* strm_;
};

class Logger : public HTTPServer::Logger {
	 public:
	  // constructor
	  explicit Logger() : strm_(NULL), lock_() {}
	  // destructor
	  ~Logger() {
	    if (strm_) close();
	  }
	  // open the stream
	  bool open(const char* path) {
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
	  void close() {
	    if (!strm_) return;
	    if (strm_ != &std::cout) delete strm_;
	    strm_ = NULL;
	  }
	  // process a log message.
	  void log(Kind kind, const char* message) {
	    if (!strm_) return;
	    char date[48];
	    datestrwww(kc::nan(), kc::INT32MAX, 6, date);
	    const char* kstr = "MISC";
	    switch (kind) {
	      case HTTPServer::Logger::DEBUG: kstr = "DEBUG"; break;
	      case HTTPServer::Logger::INFO: kstr = "INFO"; break;
	      case HTTPServer::Logger::SYSTEM: kstr = "SYSTEM"; break;
	      case HTTPServer::Logger::ERROR: kstr = "ERROR"; break;
	    }
	    lock_.lock();
	    *strm_ << date << ": [" << kstr << "]: " << message << "\n";
	    strm_->flush();
	    lock_.unlock();
	  }
	 private:
	  std::ostream* strm_;
	  kc::Mutex lock_;
	};
}
#endif