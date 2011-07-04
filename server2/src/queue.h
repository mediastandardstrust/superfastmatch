#ifndef _SFMQUEUE_H                       // duplication check
#define _SFMQUEUE_H

#include <map>
#include <iomanip>
#include <sstream>
#include <deque>
#include <registry.h>
#include <document.h>
#include <command.h>

namespace superfastmatch
{		
	class Queue{
	private:
		const Registry& registry_;
	public:
		Queue(Registry& registry);
	
		uint64_t add_document(const uint32_t doc_type,const uint32_t doc_id,const string& content,bool associate);
		uint64_t delete_document(const uint32_t& doc_type,const uint32_t& doc_id);
		
		bool process();
		bool purge();
		
		void toString(stringstream& s);		
		void toString(const uint64_t queue_id,stringstream& s);
	};

}

#endif