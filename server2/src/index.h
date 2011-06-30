#ifndef _SFMINDEX_H                       // duplication check
#define _SFMINDEX_H

#include <kcmap.h>
#include <common.h>
#include <algorithm>
#include <tr1/unordered_map>
#include <cstring>
#include <registry.h>
#include <command.h> 
// #include <posting.h>

namespace superfastmatch
{
	class Document;
	
	// Optimised to be reusable for the whole merge and prevent any new-ing or delete-ing
	// Stopped using unordered_map as it was heavier than merging vectors
	// Horrible but fast!
	class IndexLine{
	private:
		vector<uint32_t> existing_; // Stackoverflow?
		vector<uint32_t> merged_; // Stackoverflow?
		vector<uint32_t>::iterator existing_end_;
		vector<uint32_t>::iterator merged_end_;
	
 	public:
		uint32_t in_length;
		uint32_t out_length;
		uint32_t max_length;
		char* in;
		char* out;

		IndexLine(const uint32_t max_line_length);		
		~IndexLine();

		//Returns true if merge results in a change
		bool merge(const vector<uint32_t>& new_docs);
		bool remove(const vector<uint32_t>& new_docs);

	private:
		void read_existing();
		bool write_merged();
		
	};
	
	class Index
	{
	private:
		typedef vector<uint32_t> docids_vector;
		typedef unordered_map<uint32_t,docids_vector> hashes_map;
		
		const Registry& registry_;
		IndexLine line_;
		
	public:
		Index(const Registry& registry);
		~Index();		

		bool batch(deque<Command*>& docs);

	private:
		void merge(kc::TinyHashMap& hashes,bool remove);
	};
}
#endif