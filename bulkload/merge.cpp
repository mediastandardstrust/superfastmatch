#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <queue>
#include <map>
#include <stdlib.h>
#include <signal.h>

#include <kchashdb.h>

#include "Poco/Glob.h"
#include "Poco/BinaryWriter.h"
#include "Poco/LogStream.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FormattingChannel.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/Message.h"
#include "Poco/NumberFormatter.h"
#include "Poco/Timestamp.h"
#include "Poco/String.h"

#include "UInt64File.h"

using Poco::Glob;
using Poco::BinaryWriter;
using Poco::Logger;
using Poco::PatternFormatter;
using Poco::FormattingChannel;
using Poco::ConsoleChannel;
using Poco::Message;
using Poco::LogStream;
using Poco::NumberFormatter;
using Poco::Timestamp;
using Poco::cat;

using namespace std;
using namespace kyotocabinet;

// const int DB_COUNT=32L;
// const int HASH_WIDTH=32;
// const uint64_t BUCKET_COUNT=(1L<<HASH_WIDTH)-1;
// const int SLICE_SIZE=((1L<<HASH_WIDTH)-1)/DB_COUNT;
// const int BUCKET_COUNT=(1L<<(HASH_WIDTH+2))/DB_COUNT;

typedef priority_queue<UInt64File*,vector<UInt64File*>,UInt64Compare> Queue;

Queue getQueue(string path, uint64_t hash_width){
	set<string> files;
	Glob::glob(path, files);
	
	set<string>::iterator it = files.begin(); 
	Queue queue;
	
	for (; it != files.end(); ++it){
		UInt64File* file = new UInt64File(it->c_str(),hash_width);
		queue.push(file);
		cout <<"Opening :" << it->c_str() <<endl;
	}
	return queue;
}

TreeDB* open_db(string filename, uint64_t buckets,uint64_t map_size, uint64_t page_cache){
	TreeDB* db = new TreeDB();
	db->tune_buckets(buckets);
	db->tune_map(map_size);
	db->tune_page_cache(page_cache);
	// db->tune_alignment(0);
	db->tune_page(16384);
	db->tune_options(HashDB::TLINEAR);
	cout <<"Opening: " << filename <<" with bucket count of: "<< buckets <<endl;
	if (!db->open(filename, HashDB::OWRITER | HashDB::OCREATE | HashDB::OTRUNCATE)) {
		cout << "open error: " << db->error().name() << endl;
	}
	return db;
}

void close_db(TreeDB* db){
	cout << "Closing: "<< db->path() <<endl;
	if (!db->close()) {
		cout << "close error: " << db->error().name() << endl;
	}
	delete db;
}

void status_db(TreeDB* db){
	map<string,string> status;
	db->status(&status);
	map<string,string>::iterator status_it = status.begin();
	while(status_it!=status.end()){
		cout << status_it->first << ":" << status_it->second << ", ";
		status_it++;
	}
	cout <<endl;	
}

int main(int argc, char** argv)
{   
	TreeDB* db;
	string path = argv[1];
	string index = argv[2];
	uint64_t hash_width;
	uint64_t buckets;
	uint64_t map_size;
	uint64_t page_cache;
	
	stringstream(argv[3]) >> hash_width;
	stringstream(argv[4]) >> buckets;
	stringstream(argv[5]) >> map_size;
	stringstream(argv[6]) >> page_cache;
	
	// ProfilerStart("merge.prof");
	FormattingChannel* pFCConsole = new FormattingChannel(new PatternFormatter("%Y-%m-%d %H:%M:%S.%c %t"));
	pFCConsole->setChannel(new ConsoleChannel);
	pFCConsole->open();
	LogStream lstr(Logger::create("ConsoleLogger", pFCConsole, Message::PRIO_INFORMATION));
	
	Queue queue=getQueue(path,hash_width);
	
	uint64_t counter=0;
	
	uint64_t currentHash=queue.top()->getHash();
	uint64_t previousHash=queue.top()->getHash();
	
	db = open_db(index,buckets,map_size,page_cache);
	
	ostringstream documents;
	BinaryWriter documentWriter(documents); 
	
	ostringstream hash;
	BinaryWriter hashWriter(hash,BinaryWriter::BIG_ENDIAN_BYTE_ORDER);
	
	Timestamp start;
	
	while(!queue.empty()){
		UInt64File* top = queue.top();
		queue.pop();
		currentHash = top->getHash();
		if(currentHash!=previousHash){
			hashWriter << (Poco::UInt32)previousHash;
			// cout << hash.str() << ":" << documents.str() <<endl;
			db->set(hash.str(),documents.str());
			hash.str("");
			documents.str("");
		}
		previousHash=currentHash;
		documentWriter << top->getDocument();
		// documentWriter.write7BitEncoded(top->getDocument());
		if (counter%10000000==0){
			lstr << counter << " " << *top << " " << start.elapsed()/float((counter>0)?counter:1)<< endl;	
		}
		if (top->next()){
			queue.push(top);
		}
		counter++;
	}
	close_db(db);
	// ProfilerStop();
}
