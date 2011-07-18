#ifndef _SFMLOGGER_H                       // duplication check
#define _SFMLOGGER_H

#include <kthttp.h>
#include <common.h>

namespace superfastmatch{

class Logger : public kt::HTTPServer::Logger {
	private:
	  	std::ostream* strm_;
	  	kc::Mutex lock_;
	public:
		explicit Logger();
		  	
		~Logger();
	
	  	// open the stream
		bool open(const char* path);
		
		void close();
 	  	
		// process a log message.
		void log(Kind kind, const char* message);
	};
}
#endif