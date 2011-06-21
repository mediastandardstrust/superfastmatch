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
		uint32_t max_batch_count;
		double timeout;
		string map_reduce_path;		
		kyotocabinet::PolyDB* documentDB;
		kyotocabinet::PolyDB* indexDB;
		kyotocabinet::PolyDB* associationDB;
		kyotocabinet::PolyDB* queueDB;
		kyotocabinet::PolyDB* miscDB;
		
		Registry(const string& filename){
			//Todo load config from file
			window_size=15;
			thread_count=8;
			max_line_length=1<<16;
			max_hash_count=1<<24;
			max_batch_count=100;
			timeout=1.0;
			map_reduce_path="";
			documentDB = new kyotocabinet::PolyDB();
			documentDB->open("document.kct#bnum=100000#zcomp=zlib");
			indexDB = new kyotocabinet::PolyDB();
			// indexDB->open("index.kct#bnum=400000000#opts=l#msiz=6g#pccap=1g");
			indexDB->open("index.kct#bnum=1000000#opts=l#msiz=3g#pccap=1g");
			associationDB = new kyotocabinet::PolyDB();
			associationDB->open("association.kct#bnum=100000");
			queueDB = new kyotocabinet::PolyDB();
			queueDB->open("queue.kct#bnum=1000000#zcomp=zlib");
			miscDB = new kyotocabinet::PolyDB();
			miscDB->open("misc.kch");
		}
		
		~Registry(){
			documentDB->close();
			indexDB->close();
			associationDB->close();
			queueDB->close();
			miscDB->close();
			delete documentDB;
			delete indexDB;
			delete associationDB;
			delete queueDB;
			delete miscDB;
		}
	};
}
#endif