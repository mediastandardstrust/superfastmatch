#ifndef _SFMPOSTING_H                       // duplication check
#define _SFMPOSTING_H

#include <cassert>
#include <deque>
#include <map>
#include <google/sparsetable>
#include <google/sparse_hash_map>
#include <common.h>
#include <leveldb/db.h>
#include <leveldb/cache.h>
#include <leveldb/options.h>
#include <registry.h>
#include <document.h>

using google::sparsetable;  
using namespace leveldb;
using namespace std;

namespace superfastmatch
{
	class Posting{
	private:
		const Registry& registry_;
		DB* db_;
		Options options_;
		ReadOptions read_options_;
		WriteOptions write_options_;
		sparsetable<uint32_t> grouper_;
		
	public:
		Posting(const Registry& registry);
		~Posting();
		
		bool batch(deque<Document*>& docs,bool remove);
	};
}

#endif