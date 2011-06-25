#ifndef _SFMREGISTRY_H                       // duplication check
#define _SFMREGISTRY_H

#include <string>
#include <map>
#include <common.h>
#include <kchashdb.h>
#include <histogram.h>

using namespace std;
using namespace kyototycoon;

namespace superfastmatch{
	class Registry
	{
	public:
		uint32_t window_size;
		uint32_t hash_width;
		uint32_t thread_count;
		uint32_t max_line_length; //Needs to be aware of max stack size for platform
		uint32_t max_hash_count;
		uint32_t max_batch_count;
		double timeout;
		string postings_path;		
		kc::PolyDB* documentDB;
		kc::TreeDB* indexDB;
		kc::PolyDB* associationDB;
		kc::PolyDB* queueDB;
		kc::PolyDB* miscDB;
		
		Registry(const string& filename);
		~Registry();
		
		friend std::ostream& operator<< (std::ostream& stream, Registry& registry);
	
	private:
		void status(std::ostream& s, kc::TreeDB* db);
	};
}
#endif