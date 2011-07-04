#ifndef _SFMPOSTING_H                       // duplication check
#define _SFMPOSTING_H

#include <cassert>
#include <deque>
#include <map>
#include <google/sparsetable>
#include <google/sparse_hash_map>
#include <kcthread.h>
#include <common.h>

using google::sparsetable;  
using namespace std;

namespace superfastmatch
{
	// Forward Declarations
	class Document;
	class Registry;
	class Command;
	class PostingSlot;
	
	typedef map<uint32_t,vector<uint32_t> > histogram_t;

	class TaskPayload{
	public:
		enum TaskOperation{
			AddDocument,
			DeleteDocument
		};
	private:
		Document* document_;
		TaskOperation operation_;
		kc::AtomicInt64 slots_left_;
	public:
		TaskPayload(Document* document,TaskOperation operation,uint32_t slots);
		~TaskPayload();
		
		uint64_t markSlotFinished();
		TaskOperation getTaskOperation();
		Document* getDocument();
	};

	class PostingTaskQueue : public kc::TaskQueue{
	public:
		explicit PostingTaskQueue();
	private:
		void do_task(Task* task);
	};

	class PostingTask : public kc::TaskQueue::Task{
	private:
		PostingSlot* slot_;
		TaskPayload* payload_;
	public:
		explicit PostingTask(PostingSlot* slot,TaskPayload* payload);
		~PostingTask();
		
		PostingSlot* getSlot();
		TaskPayload* getPayload();
	};

	class PostingSlot{
	private:
		class PostLine{
		private:
			static const uint16_t DEFAULT_BUCKET_LENGTH = 16;
			char* bucket_;
		public:
			PostLine();
			~PostLine();
			
			uint32_t decode(vector<uint32_t>& line);
			uint32_t encode(const vector<uint32_t>& line,char* out);
			void commit(const char* out,const uint32_t length, const bool allocate);
		};
		
		const Registry& registry_;
		const uint32_t slot_number_;
		hash_t offset_;
		hash_t span_;
		sparsetable<PostLine,48> index_;
		vector<uint32_t> line_;
		char* out_;
		histogram_t doc_counts_;
		histogram_t gap_counts_;
		kc::RWLock index_lock_;
		kc::RWLock doc_counts_lock_;
		kc::RWLock gap_counts_lock_;
		PostingTaskQueue queue_;
		
	public:
		PostingSlot(const Registry& registry,uint32_t slot_number);
		~PostingSlot();
		
		// Returns number of items in queue
		bool alterIndex(Document* doc,TaskPayload::TaskOperation operation);
		void mergeHistogram(histogram_t& histogram);
		uint32_t fill_list_dictionary(TemplateDictionary* dict,hash_t start);
		uint64_t addTask(TaskPayload* payload);
		uint32_t getTaskCount();
	};

	class Posting{
	private:		
		const Registry& registry_;
		vector<PostingSlot*> slots_;
		uint32_t doc_count_;
		uint32_t hash_count_;
		bool ready_;
		
		// Folling three methods return the current queue length for all slots combined		
		uint64_t alterIndex(Document* doc,TaskPayload::TaskOperation operation);
		uint64_t addDocument(Document* doc);
		uint64_t deleteDocument(Document* doc);
		
	public:
		Posting(const Registry& registry);
		~Posting();
		
		bool init();
		// void compareDocument(Document* doc,unordered_map<uint32_t,<unordered_map<uint32_t,uint32_t> >* results);
		bool addDocuments(vector<Command*> commands);
		bool deleteDocuments(vector<Command*> commands);
		bool isReady();
		
		void fill_list_dictionary(TemplateDictionary* dict,hash_t start);
		void fill_histogram_dictionary(TemplateDictionary* dict);		
	};
}

#endif