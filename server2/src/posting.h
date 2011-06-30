#ifndef _SFMPOSTING_H                       // duplication check
#define _SFMPOSTING_H

#include <cassert>
#include <deque>
#include <map>
#include <google/sparsetable>
#include <google/sparse_hash_map>
#include <ctemplate/template.h>  
#include <kcthread.h>
#include <common.h>

using google::sparsetable;  
using namespace std;

namespace superfastmatch
{
	// Forward Declarations
	class Document;
	class Registry;
	
	// We choose a size of 32 so that all hash widths can be slotted 
	// by either 2,4 or 8 threads without any locking
	// Eg. hash_width=25 then max_hash_count=33554432
	//     33554432/4/32=262144
	// With the default size of 48 we get a fractional value of 174762.6666...
	// which would mean that more than one thread could resize a sparsegroup
	typedef sparsetable<hash_t,32> grouper_t;
	typedef map<uint32_t,vector<uint32_t> > histogram_t;

	class Posting{
	private:		
		const Registry& registry_;
		uint32_t doc_count_;
		uint32_t hash_count_;
		vector<hash_t> slots_;
		grouper_t grouper_;
		histogram_t histogram_;
		kc::Mutex hist_lock_;

		//Cursor per thread instead?

		// class PostingQueue : public kc::TaskQueue {
		// 	   	public:
		// 	    	class PostingTask : public Task {
		// 	      		friend class PostingQueue;
		// 	     	public:
		// 	      		explicit PostingTask(Document* doc,AtomicInt64* state):
		// 	          	doc_(doc){}
		// 	     	private:
		// 		Document* doc_;
		// 		AtomicInt64* state_;
		// 	    	};
		// 	    explicit PostingQueue(uint32_t slot):
		// slot_(slot){}
		// 	   	private:
		// 	uint32_t slot_;
		// 	    	void do_task(Task* task) {
		// 	      		PostingTask* ptask = (PostingTask*)task;
		// 		addDocument(ptask.doc_,slot_);
		// 		ptask.state_++;
		// 	      		delete ptask;
		// 	    	}
		// };
		
	public:
		Posting(const Registry& registry);
		~Posting();
		
		bool addDocument(Document* doc,uint32_t slot=0);
		bool deleteDocument(Document* doc);
		
		friend std::ostream& operator<< (std::ostream& stream, Posting& posting);
	};
}

#endif