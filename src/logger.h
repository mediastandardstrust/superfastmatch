#ifndef _SFMLOGGER_H                       // duplication check
#define _SFMLOGGER_H

#include <kthttp.h>
#include <common.h>

namespace superfastmatch{

  class Logger : public kt::HTTPServer::Logger {
  private:
    bool debug_;
    std::ostream* strm_;
    kc::Mutex lock_;
  public:
    explicit Logger(const bool debug);

    ~Logger();

    // open the stream
    bool open(const char* path);
    void close();

    // process a log message and clear stringstream.
    void log(Kind kind, stringstream* s);
    
    // process a log message.
    void log(Kind kind, const char* message);
    
  };
}
#endif