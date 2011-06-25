#include "posting.h"

namespace superfastmatch
{
	Posting::Posting(const Registry& registry):
	registry_(registry)
	{
		grouper_.resize((1L<<registry.hash_width)-1);
		cout << grouper_.max_size() << ":" <<grouper_.size() << endl;
		options_.create_if_missing=true;
		options_.compression = kNoCompression;
	  	options_.block_cache = leveldb::NewLRUCache(100 * 1048576);  // 100MB cache
		Status status = DestroyDB(registry.postings_path,options_);
		assert(status.ok());
		status = DB::Open(options_,registry_.postings_path, &db_);
		assert(status.ok());
	}
	Posting::~Posting(){
		delete db_;
	}

	bool Posting::batch(deque<Document*>& docs,bool remove){
		std::sort(docs.begin(),docs.end());
		uint64_t append_count=0;
		while (!docs.empty()){
			Document* doc = docs.front();
			cout <<"Posting: " << *doc << endl;
			for (Document::hashes_vector::const_iterator it=doc->unique_sorted_hashes().begin(),ite=doc->unique_sorted_hashes().end();it!=ite;++it){
				if (!grouper_.test(*it)){
					grouper_.set(*it,0);
				}
				// static_cast<deque<uint32_t>* >(grouper_[*it])->push_back(doc->doctype());
				// static_cast<deque<uint32_t>* >(grouper_[*it])->push_back(doc->docid());
				grouper_[*it]=grouper_[*it]+1;
				append_count++;
			}
			// if (grouper_.num_nonempty()>registry_.max_hash_count){
			// 	cout << "Hash count limit reached: " << grouper_.num_nonempty() << " > " << registry_.max_hash_count << " Appends: " << append_count<< endl;
			// 	append_count=0;
			// // 		merge(hashes,remove);
			// 	grouper_.clear();
			// }
			doc->clear();
			docs.pop_front();
		}
	
		map<uint32_t,uint32_t> histogram;
		for (sparsetable<uint32_t>::const_nonempty_iterator it=grouper_.nonempty_begin();it!=grouper_.nonempty_end();++it){
			histogram[*it]++;
		}
		for (map<uint32_t,uint32_t>::const_iterator it=histogram.begin();it!=histogram.end();++it){
			cout << it->first << ":" << it->second <<endl;
		} 
	
		cout << "End of batch reached: " << grouper_.num_nonempty() << " < " << registry_.max_hash_count << " Appends: " << append_count<< endl;
		append_count=0;
		// merge(hashes,remove);
		grouper_.clear();
		return true;
	}
}