#ifndef _SFMREGISTRY_H                       // duplication check
#define _SFMREGISTRY_H

#include <kcpolydb.h>
#include <string>
#include <common.h>

using namespace std;
using namespace kyototycoon;

namespace superfastmatch{
	struct Registry
	{
		uint32_t window_size;
		uint32_t thread_count;
		uint32_t max_line_length; //Needs to be aware of max stack size for platform
		uint32_t max_hash_count;
		double timeout;
		string map_reduce_path;		
		kyotocabinet::PolyDB* documentDB;
		kyotocabinet::PolyDB* indexDB;
		kyotocabinet::PolyDB* associationDB;
		kyotocabinet::PolyDB* jobDB;
		kyotocabinet::PolyDB* queueDB;
		
		Registry(const string& filename){
			//Todo load config from file
			window_size=15;
			thread_count=8;
			max_line_length=1<<16;
			max_hash_count=1<<24;
			timeout=1.0;
			map_reduce_path="";
			documentDB = new kyotocabinet::PolyDB();
			documentDB->open("document.kct#bnum=100000#zcomp=zlib#msiz=1g");
			indexDB = new kyotocabinet::PolyDB();
			indexDB->open("index.kct#bnum=1000000#psiz=32768#msiz=4g#pccap=4g");
			associationDB = new kyotocabinet::PolyDB();
			associationDB->open("association.kct#bnum=100000");
			jobDB = new kyotocabinet::PolyDB();
			jobDB->open("job.kct#bnum=100000");
			queueDB = new kyotocabinet::PolyDB();
			queueDB->open("hashqueue.kct#bnum=1000000");
		}
		
		~Registry(){
			documentDB->close();
			indexDB->close();
			associationDB->close();
			jobDB->close();
			delete documentDB;
			delete indexDB;
			delete associationDB;
			delete jobDB;
			delete queueDB;
		}
	};
}
#endif