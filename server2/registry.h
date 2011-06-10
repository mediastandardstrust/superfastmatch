#ifndef _SFMREGISTRY_H                       // duplication check
#define _SFMREGISTRY_H

#include <kttimeddb.h>
#include <string>
#include <common.h>

using namespace std;
using namespace kyototycoon;

namespace superfastmatch{
	struct Registry
	{
		uint32_t window_size;
		uint32_t thread_count;
		double timeout;
		string map_reduce_path;
		TimedDB* documentDB;
		TimedDB* indexDB;
		TimedDB* associationDB;
		TimedDB* jobDB;
		
		Registry(const string& filename){
			//Todo load config from file
			window_size=15;
			thread_count=8;
			timeout=1.0;
			map_reduce_path="";
			documentDB = new TimedDB();
			documentDB->open("document.kct#bnum=100000#ktopts=p#zcomp=zlib");
			indexDB = new TimedDB();
			indexDB->open("index.kct#bnum=10000000#msiz=1g#pccap=1g#ktopts=p");
			associationDB = new TimedDB();
			associationDB->open("association.kct#bnum=100000#ktopts=p");
			jobDB = new TimedDB();
			jobDB->open("job.kct#bnum=100000#ktopts=p");
		}
		
		~Registry(){
			documentDB->close();
			indexDB->close();
			associationDB->close();
			delete documentDB;
			delete indexDB;
			delete associationDB;
			delete jobDB;
		}
	};
}
#endif