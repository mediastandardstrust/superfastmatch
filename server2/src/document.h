#ifndef _SFMDOCUMENT_H                       // duplication check
#define _SFMDOCUMENT_H

#include <vector>
#include <bitset>
#include <iostream>
#include <string>
#include <map>
#include <algorithm>
#include <common.h>
#include <ktutil.h>
#include <registry.h>

namespace superfastmatch
{		
	class Document
	{
	public:
		typedef std::vector<hash_t> hashes_vector;
		typedef std::bitset<(1<<24)> hashes_bloom;
		typedef std::map<std::string,std::string> content_map;
		
	private:
		uint32_t doctype_;
		uint32_t docid_;
		const Registry& registry_;
		std::string* key_;
		std::string* content_;
		content_map* content_map_;
		hashes_vector* hashes_;
		hashes_vector* unique_sorted_hashes_;
		hashes_bloom* bloom_;

		
	public:
		Document(const uint32_t doctype,const uint32_t docid,const char* content,const Registry& registry);
		Document(string& key,const Registry& registry);
		
		~Document();
		
		void clear();
		//Returns false if document already exists
		bool save();
		bool load();
		bool remove();
		void serialize(stringstream& s);

		hashes_vector& hashes();
		hashes_vector& unique_sorted_hashes();
		hashes_bloom& bloom();
		content_map& content();
		string& text();
		string& key();
		uint64_t index_key();
		
		uint32_t windowsize();
		uint32_t doctype();				
		uint32_t docid();
			
		friend std::ostream& operator<< (std::ostream& stream, Document& document);
		
		friend bool operator< (Document& lhs,Document& rhs);
	};
	
}//namespace Superfastmatch

#endif                                   // duplication check