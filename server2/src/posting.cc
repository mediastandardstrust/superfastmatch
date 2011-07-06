#include "posting.h"

//Is this naughty?
#include "document.h"
#include "command.h"
#include "registry.h"

namespace superfastmatch
{
	// -------------------
	// TaskPayload members
	// -------------------
	
	TaskPayload::TaskPayload(Document* document,TaskOperation operation,uint32_t slots):
	document_(document),operation_(operation),slots_left_(slots)
	{}
	
	TaskPayload::~TaskPayload(){
		delete document_;
	}
		
	uint64_t TaskPayload::markSlotFinished(){
		uint64_t o_slots=slots_left_;
		do{
			o_slots=slots_left_;
		}while(!slots_left_.cas(o_slots,o_slots-1));
		return o_slots;
	}
	
	TaskPayload::TaskOperation TaskPayload::getTaskOperation(){
		return operation_;
	}
	
	Document* TaskPayload::getDocument(){
		return document_;
	}
	
	// -------------------
	// PostingTask members
	// -------------------
	
	PostingTask::PostingTask(PostingSlot* slot,TaskPayload* payload):
	slot_(slot),payload_(payload)
	{}
	
	PostingTask::~PostingTask(){
		// The task is complete so delete the payload
		if (payload_->markSlotFinished()==1){
			delete payload_;
		}
	}
	
	PostingSlot* PostingTask::getSlot(){
		return slot_;
	}
	
	TaskPayload* PostingTask::getPayload(){
		return payload_;
	}
	
	// ------------------------
	// PostingTaskQueue members
	// ------------------------
	
	PostingTaskQueue::PostingTaskQueue(){}
	
	void PostingTaskQueue::do_task(Task* task) {
      	PostingTask* ptask = (PostingTask*)task;
		Document* doc = ptask->getPayload()->getDocument();
		PostingSlot* slot = ptask->getSlot();
		slot->alterIndex(doc,ptask->getPayload()->getTaskOperation());
		delete ptask;
    }

	// ----------------
	// PostLine members
	// ----------------
	
	// TODO Consider an arena!
	PostingSlot::PostLine::PostLine(const size_t size):
	bucket_(new char[size])
	{
		memset(bucket_,0,size);
	}
	
	// This is cheeky
	// PostingSlot::PostLine::PostLine(const PostLine& copy):
	// bucket_(copy.bucket_){}
	// 
	// PostingSlot::PostLine & PostingSlot::PostLine::operator=(const PostingSlot::PostLine& other){
	// 	if (this != &other)
	//         {
	//             char* bucket_=
	//             std::copy(other.bucket_, other.bucket_ + strlen(other.bucket_)+1, new_bucket_);
	//             delete [] bucket_;
	//             bucket_ = nebucket_;
	//         }
	//         return *this;
	// }
	// 
	// PostingSlot::PostLine::~PostLine(){}

	void PostingSlot::PostLine::clear(){
		if (bucket_!=NULL){
			delete[] bucket_;
		}
		bucket_=NULL;
	}

	void PostingSlot::PostLine::commit(const char* out,const uint32_t length) const{
		memcpy(bucket_,out,length);
	}
	
	uint32_t PostingSlot::PostLine::decode(vector<uint32_t>& line) const{
		uint32_t offset=0;
		uint64_t docs_length;
		uint64_t previous;
		uint64_t value;
		line.resize(0);
		while (bucket_[offset]!=0){
			// Read doc type
			offset+=kc::readvarnum(bucket_+offset,5,&value);
			line.push_back(value);
			// Read doc type sequence length
			offset+=kc::readvarnum(bucket_+offset,5,&docs_length);
			line.push_back(docs_length);
			// Read the delta encoded docids
			previous=0;
			for (uint32_t i=0;i<docs_length;i++){
				offset+=kc::readvarnum(bucket_+offset,5,&value);
				line.push_back(value+previous);
				previous+=value;
			}
		}
		// Include the terminating 0
		return offset+1;
	}
	
	uint32_t PostingSlot::PostLine::encode(const vector<uint32_t>& line,char* out)const{
		uint32_t offset=0;
		uint32_t index=0;
		uint64_t docs_length;
		uint64_t previous;
		uint64_t value;
		while (index<line.size()){
			// Write doc type
			value=line[index++];
			offset+=kc::writevarnum(out+offset,value);
			// Write doc type sequence length
			docs_length=line[index++];
			offset+=kc::writevarnum(out+offset,docs_length);
			// Write the delta encoded docids
			previous=0;
			for (uint32_t i=0;i<docs_length;i++){
				value=line[index]-previous;
				offset+=kc::writevarnum(out+offset,value);
				previous=line[index++];
			}
		}
		offset+=kc::writevarnum(out+offset,0);
		return offset;
	}

	// -------------------
	// PostingSlot members
	// -------------------
	
	PostingSlot::PostingSlot(const Registry& registry,uint32_t slot_number):
	registry_(registry),slot_number_(slot_number),
	offset_((registry.max_hash_count/registry.slot_count)*slot_number), // Offset for sparsetable insertion
	span_(registry.max_hash_count/registry.slot_count),	// Span of slot, ie. ignore every hash where (hash-offset)>span
	out_(new char[registry.max_line_length*5]),
	index_(registry.max_hash_count/registry.slot_count)
	{
		line_.reserve(registry.max_line_length);
		// 1 thread per slot, increase slot_number to get more threads!
		queue_.start(1);
	}
	
	PostingSlot::~PostingSlot(){
		queue_.finish();
		index_.clear();
	}
	
	void PostingSlot::debug(const char* prefix,hash_t hash){
		stringstream s;
		s << prefix << hash << ": ";
		for (vector<uint32_t>::iterator it=line_.begin(),ite=line_.end();it!=ite;++it){
			s << *it << ":";
		}
		registry_.logger->log(superfastmatch::Logger::INFO,s.str().c_str());
	}
	
	bool PostingSlot::alterIndex(Document* doc,TaskPayload::TaskOperation operation){
		if (doc_counts_[doc->doctype()].size()==0){
			doc_counts_lock_.lock_writer();
			doc_counts_[doc->doctype()].resize(registry_.max_line_length);
			doc_counts_[doc->doctype()][0]=index_.size();
			doc_counts_lock_.unlock();
		}
		hash_t hash;
		uint32_t doc_count;
		uint32_t incoming_length;
		uint32_t outgoing_length;
		// Where hash width is below 32 we will get duplicates per document
		// We discard them with a no operation 
		vector<uint32_t>::iterator line_iterator;
		vector<uint32_t>::iterator doc_count_marker;
		vector<uint32_t>::iterator merge_end;
		index_lock_.lock_writer();
		for (Document::hashes_vector::const_iterator it=doc->unique_sorted_hashes().begin(),ite=doc->unique_sorted_hashes().end();it!=ite;++it){
			hash = ((*it>>registry_.hash_width)^(*it&registry_.hash_mask))-offset_;
			if (hash<span_){
				bool noop=false;
				if (!index_.test(hash)){
					PostLine* new_line = new PostLine();
					index_.set(hash,*new_line);
				}
				PostLine* posting_line=&index_[hash];
				incoming_length=posting_line->decode(line_);
				// debug("Before: ",hash);
				line_iterator=line_.begin();
				switch (operation){
					case TaskPayload::AddDocument:
						while (true){
							// Insert before
							if ((*line_iterator>doc->doctype()) || (line_iterator==line_.end())){
								line_iterator=line_.insert(line_iterator,doc->docid());
								line_iterator=line_.insert(line_iterator,1);
								line_iterator=line_.insert(line_iterator,doc->doctype());
								doc_count=1;
								break;
							}
							// Merge
							else if (*line_iterator==doc->doctype()){
								// Move to doc type length
								line_iterator++;
								// And record the new doc count position
								doc_count_marker=line_iterator;
								merge_end=line_iterator+*line_iterator+1;
								// And insert the item in sorted order
								while(true){
									line_iterator++;
					 				if (line_iterator==line_.end()){
										line_.push_back(doc->docid());	
										*doc_count_marker+=1;
										doc_count=*doc_count_marker;
										break;
									}else if(line_iterator==merge_end){
										line_.insert(merge_end,doc->docid());	
										*doc_count_marker+=1;
										doc_count=*doc_count_marker;
										break;
									}else if ((*line_iterator>doc->docid())){
										line_.insert(line_iterator,doc->docid());
										*doc_count_marker+=1;
										doc_count=*doc_count_marker;
										break;
									}else if (*line_iterator==doc->docid()){ 	// Check for dupes
										noop=true;
										break;
									};
									
								}
								break;
							}else{
								// Move to doc type length 
								line_iterator++;
								// Jump to next doc type
								line_iterator+=(*line_iterator+1);
							}
						}
						if (!noop){
							doc_counts_[doc->doctype()][doc_count-1]--;
							doc_counts_[doc->doctype()][doc_count]++;	
						}
						break;
					case TaskPayload::DeleteDocument:
						doc_counts_[doc->doctype()][doc_count]--;
						doc_counts_[doc->doctype()][doc_count-1]++;
						break;
				}
				// debug("After: ",hash);
				// Decide how to allocate memory
				if (!noop){
					outgoing_length=posting_line->encode(line_,out_);
					if ((outgoing_length/16)!=(incoming_length/16)){
						size_t size=((outgoing_length/16)+1)*16;
						posting_line->clear();
						PostLine* new_line = new PostLine(size);
						index_.set(hash,*new_line);
						new_line->commit(out_,outgoing_length);	
					}else{
						posting_line->commit(out_,outgoing_length);	
					}
				}
			}
		}
		index_lock_.unlock();
		return true;
	}
	
	bool PostingSlot::searchIndex(const vector<hash_t>& hashes,search_t& results){
		vector<uint32_t> line;
		vector<uint32_t>::const_iterator line_cursor;
		vector<uint32_t>::const_iterator doc_it;
		vector<uint32_t>::const_iterator doc_ite;
		uint32_t doc_type;
		hash_t hash;
		index_lock_.lock_reader();
		for (vector<hash_t>::const_iterator it=hashes.begin(),ite=hashes.end();it!=ite;++it){
			hash = ((*it>>registry_.hash_width)^(*it&registry_.hash_mask))-offset_;
			if ((hash<span_)&&index_.test(*it)){
				PostLine post_line=index_.unsafe_get(*it);	
				post_line.decode(line);
				line_cursor=line.begin();
				while(line_cursor!=line.end()){
					doc_type=*line_cursor;
					doc_it=line_cursor+2;
					doc_ite=doc_it+*(line_cursor+1);
					std::copy(doc_it,doc_ite,back_inserter(results[hash][doc_type]));
					line_cursor=doc_ite;
				}
			}
		}
		index_lock_.unlock();
		return true;
	}
	
	uint64_t PostingSlot::addTask(TaskPayload* payload){
		PostingTask* task = new PostingTask(this,payload);
		return queue_.add_task(task);
	}
	
	uint32_t PostingSlot::getTaskCount(){
		return queue_.count();
	}
	
	uint32_t PostingSlot::fill_list_dictionary(TemplateDictionary* dict,hash_t start){
		uint32_t count=0;
		hash_t hash=(offset_>start)?0:start-offset_;
		if (hash<span_){
			vector<hash_t> existing;
			index_lock_.lock_reader();
			for(;hash<span_;hash++){
				if(index_.test(hash)){
					existing.push_back(hash);
					count++;
					if(count>registry_.page_size){
						break;
					}
				}
			}
			index_lock_.unlock();
			search_t results;
			searchIndex(existing,results);
			for (search_t::iterator it=results.begin(),ite=results.end();it!=ite;++it){
				for (search_line_t::iterator it2=it->second.begin(),ite2=it->second.end();it2!=ite2;++it2){
					TemplateDictionary* posting_dict = dict->AddSectionDictionary("POSTING");
					if (it2==it->second.begin()){
						TemplateDictionary* hash_dict = posting_dict->AddSectionDictionary("HASH");				
						hash_dict->SetIntValue("HASH",it->first);
						hash_dict->SetIntValue("DOC_TYPE_COUNT",it->second.size());
					}
					uint32_t previous=0;
					for (uint32_t i=0;i<it2->second.size();i++){
						posting_dict->SetIntValue("DOC_TYPE",it2->first);	
						TemplateDictionary* doc_id_dict = posting_dict->AddSectionDictionary("DOC_IDS");
						doc_id_dict->SetIntValue("DOC_ID",it2->second[i]);	
						TemplateDictionary* doc_delta_dict = posting_dict->AddSectionDictionary("DOC_DELTAS");
						doc_delta_dict->SetIntValue("DOC_DELTA",it2->second[i]-previous);
						previous=it2->second[i];
					}
				}
			}
		}
		return count;
	}
	
	void PostingSlot::mergeHistogram(histogram_t& histogram){
		doc_counts_lock_.lock_reader();
		for (histogram_t::iterator it=doc_counts_.begin(),ite=doc_counts_.end();it!=ite;++it){
			if (histogram[it->first].size()==0){
				histogram[it->first].resize(registry_.max_line_length);
			}
			for (uint32_t i=0;i<(it->second).size();i++){
				histogram[it->first][i]+=doc_counts_[it->first][i];
			}
		}
		doc_counts_lock_.unlock();
	}
	
	// ---------------
	// Posting members
	// ---------------
	
	Posting::Posting(const Registry& registry):
	registry_(registry),doc_count_(0),hash_count_(0),ready_(false)
	{
		for (uint32_t i=0;i<registry.slot_count;i++){
			slots_.push_back(new PostingSlot(registry,i));
		}
	}
	
	Posting::~Posting(){
		for (uint32_t i=0;i<slots_.size();i++){
			delete slots_[i];
		}
	}
	
	bool Posting::init(){
		// Load the stored docs
		double start = kc::time();
		DocumentCursor* cursor = new DocumentCursor(registry_);
		Document* doc;
		while ((doc=cursor->getNext())!=NULL){
			if (addDocument(doc)>2000){
				cout << "Chilling with queue lengths: ";
				for (uint32_t i=0;i<registry_.slot_count;i++){
					cout << slots_[i]->getTaskCount() << ":";
				}
				cout << endl;
				// Throttle
				kc::Thread::chill();
			}
		}
		delete cursor;
		for (uint32_t i=0;i<registry_.slot_count;i++){
			if (slots_[i]->getTaskCount()!=0){
				sleep(0.2);				
			}
		}
		cout << "Posting initialisation finished in: " << setiosflags(ios::fixed) << setprecision(4) << kc::time()-start << " secs" << endl;
		ready_=true;
		return ready_;
	}
	
	uint64_t Posting::alterIndex(Document* doc,TaskPayload::TaskOperation operation){
		uint64_t queue_length=0;
		TaskPayload* task = new TaskPayload(doc,operation,registry_.slot_count);
		for (uint32_t i=0;i<registry_.slot_count;i++){
			queue_length+=slots_[i]->addTask(task);
		}
		return queue_length;
	}
	
	uint64_t Posting::addDocument(Document* doc){
		cout << "Adding: " << *doc << endl;
		uint64_t queue_length=alterIndex(doc,TaskPayload::AddDocument);
		doc_count_++;
		hash_count_+=doc->unique_sorted_hashes().size();
		return queue_length;
	}
	
	uint64_t Posting::deleteDocument(Document* doc){
		cout << "Deleting: " << *doc << endl;
		uint64_t queue_length=alterIndex(doc,TaskPayload::DeleteDocument);
		doc_count_--;
		hash_count_-=doc->unique_sorted_hashes().size();
		return queue_length;
	}
	
	bool Posting::addDocuments(vector<Command*> commands){
		for (vector<Command*>::iterator it=commands.begin(),ite=commands.end();it!=ite;++it){
			if (addDocument((*it)->getDocument())>100){
				// sleep(1.0);
			}
		}
		for (uint32_t i=0;i<registry_.slot_count;i++){
			if (slots_[i]->getTaskCount()!=0){
				sleep(1.0);				
			}
		}
		return true;
	}
	
	bool Posting::deleteDocuments(vector<Command*> commands){
		for (vector<Command*>::iterator it=commands.begin(),ite=commands.end();it!=ite;++it){
			if (deleteDocument((*it)->getDocument())>100){
				// sleep(1.0);
			}
		}
		for (uint32_t i=0;i<registry_.slot_count;i++){
			if (slots_[i]->getTaskCount()!=0){
				sleep(1.0);				
			}
		}
		return true;
	}
	
	bool Posting::isReady(){
		return ready_;
	}
	
	void Posting::fill_list_dictionary(TemplateDictionary* dict,hash_t start){
		uint32_t count=0;
		for (uint32_t i=0;i<slots_.size();i++){
			count+=slots_[i]->fill_list_dictionary(dict,start);
			if (count>registry_.page_size){
				break;
			}
		}
	}

	void Posting::fill_histogram_dictionary(TemplateDictionary* dict){
		histogram_t hist;
		for (uint32_t i=0;i<slots_.size();i++){
			slots_[i]->mergeHistogram(hist);
		}
		dict->SetIntValue("DOC_COUNT",doc_count_);
		dict->SetIntValue("HASH_COUNT",hash_count_);
		dict->SetIntValue("AVERAGE_DOC_LENGTH",(doc_count_>0)?hash_count_/doc_count_:0);
		for (histogram_t::iterator it = hist.begin(),ite=hist.end();it!=ite;++it){
			dict->ShowSection("HISTOGRAM");
			dict->SetValueAndShowSection("DOC_TYPE",toString(it->first),"COLUMNS");
		}
		for (uint32_t i=0;i<500;i++){
			TemplateDictionary* rows_dict=dict->AddSectionDictionary("ROWS");
			for (histogram_t::iterator it = hist.begin(),ite=hist.end();it!=ite;++it){
				rows_dict->SetValueAndShowSection("DOC_COUNTS",toString(it->second[i]),"COLUMN");
			}
		}
		// dict->Dump();
	}
}