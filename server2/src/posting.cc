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
    uint32_t index=0;
    uint64_t docs_length;
    uint64_t doc_type;
    uint64_t previous;
    uint64_t value;
    line.resize(0);
    while (bucket_[offset]!=0){
      // Read doc type
      offset+=kc::readvarnum(bucket_+offset,5,&doc_type);
      // Read doc type sequence length
      offset+=kc::readvarnum(bucket_+offset,5,&docs_length);
      line.resize(index+docs_length+2);
      line[index++]=doc_type;
      line[index++]=docs_length;
      // Read the delta encoded docids
      previous=0;
      for (uint32_t i=0;i<docs_length;i++){
        offset+=kc::readvarnum(bucket_+offset,5,&value);
        line[index++]=value+previous;
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
  span_(registry.max_hash_count/registry.slot_count), // Span of slot, ie. ignore every hash where (hash-offset)>span
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
    hash_t hash;
    uint32_t doc_count=0;
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
      uint32_t doctype=doc->doctype();
      uint32_t docid=doc->docid();
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
        if(line_.size()<registry_.max_line_length){
          switch (operation){
            case TaskPayload::AddDocument:
              while (true){
                // Insert before
                if ((*line_iterator>doctype) || (line_iterator==line_.end())){
                  line_iterator=line_.insert(line_iterator,docid);
                  line_iterator=line_.insert(line_iterator,1);
                  line_iterator=line_.insert(line_iterator,doctype);
                  doc_count=1;
                  break;
                }
                // Merge
                else if (*line_iterator==doctype){
                  // Move to doc type length
                  line_iterator++;
                  // And record the new doc count position
                  doc_count_marker=line_iterator;
                  merge_end=line_iterator+*line_iterator+1;
                  // And insert the item in sorted order
                  while(true){
                    line_iterator++;
                    if (line_iterator==line_.end()){
                      line_.push_back(docid); 
                      *doc_count_marker+=1;
                      doc_count=*doc_count_marker;
                      break;
                    }else if(line_iterator==merge_end){
                      line_.insert(merge_end,docid);  
                      *doc_count_marker+=1;
                      doc_count=*doc_count_marker;
                      break;
                    }else if ((*line_iterator>docid)){
                      line_.insert(line_iterator,docid);
                      *doc_count_marker+=1;
                      doc_count=*doc_count_marker;
                      break;
                    }else if (*line_iterator==docid){   // Check for dupes
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
              break;
            case TaskPayload::DeleteDocument:
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
  
  bool PostingSlot::searchIndex(const vector<hash_t>& hashes,search_t& results,usage_t& usage){
    vector<uint32_t> line;
    vector<uint32_t>::const_iterator line_cursor;
    vector<uint32_t>::const_iterator doc_it;
    vector<uint32_t>::const_iterator doc_ite;
    uint32_t doc_type;
    index_lock_.lock_reader();
    for (vector<hash_t>::const_iterator it=hashes.begin(),ite=hashes.end();it!=ite;++it){
      if (index_.test(*it)){
        PostLine post_line=index_.unsafe_get(*it);  
        usage[*it+offset_]=post_line.decode(line);
        line_cursor=line.begin();
        while(line_cursor!=line.end()){
          doc_type=*line_cursor;
          doc_it=line_cursor+2;
          doc_ite=doc_it+*(line_cursor+1);
          std::copy(doc_it,doc_ite,back_inserter(results[*it+offset_][doc_type]));
          line_cursor=doc_ite;
        }
      }
    }
    index_lock_.unlock();
    return true;
  }
  
  uint32_t PostingSlot::fill_list_dictionary(TemplateDictionary* dict,hash_t start){
    index_lock_.lock_reader();
    hash_t hash=(offset_>start)?0:start-offset_;
    // Find first hash
    while (hash<span_ && !index_.test(hash)){    
      hash++;
    }
    // If not in slot break early
    if (index_.num_nonempty()==0 || hash>=span_){
        index_lock_.unlock();
        return 0;
    }
    // Scan non empty
    uint32_t count=0;
    vector<uint32_t> line;
    uint32_t doc_type;
    vector<uint32_t>::const_iterator line_cursor;
    vector<uint32_t>::const_iterator doc_it;
    vector<uint32_t>::const_iterator doc_ite;
    sparsetable<PostLine>::nonempty_iterator it=index_.get_iter(hash);
    while (it!=index_.nonempty_end() && count<registry_.page_size){
      uint32_t bytes=it->decode(line);
      uint32_t doc_type_count=0;
      line_cursor=line.begin();
      TemplateDictionary* hash_dict=NULL;
      while(line_cursor!=line.end()){
        TemplateDictionary* posting_dict = dict->AddSectionDictionary("POSTING");
        if (line_cursor==line.begin()){
           hash_dict = posting_dict->AddSectionDictionary("HASH");        
        }
        doc_type=*line_cursor;
        doc_type_count++;
        doc_it=line_cursor+2;
        doc_ite=doc_it+*(line_cursor+1);
        posting_dict->SetIntValue("DOC_TYPE",doc_type); 
        uint32_t previous=0;
        while(doc_it!=doc_ite){
          TemplateDictionary* doc_id_dict = posting_dict->AddSectionDictionary("DOC_IDS");
          doc_id_dict->SetIntValue("DOC_ID",*doc_it); 
          TemplateDictionary* doc_delta_dict = posting_dict->AddSectionDictionary("DOC_DELTAS");
          doc_delta_dict->SetIntValue("DOC_DELTA",*doc_it-previous);
          previous=*doc_it;
          doc_it++;
        }
        line_cursor=doc_ite;
        if (line_cursor==line.end()){
          hash_dict->SetIntValue("HASH",index_.get_pos(it)+offset_);
          hash_dict->SetIntValue("BYTES",bytes);
          hash_dict->SetIntValue("DOC_TYPE_COUNT",doc_type_count); 
        }
      }
      it++;
      count++;
    }
    index_lock_.unlock();
    return count;
  }
  
  void PostingSlot::fillHistograms(histogram_t& hash_hist,histogram_t& gaps_hist){
    vector<uint32_t> line;
    vector<uint32_t>::const_iterator line_cursor;
    vector<uint32_t>::const_iterator doc_it;
    vector<uint32_t>::const_iterator doc_ite;
    uint32_t doc_type;
    uint32_t doc_length;
    line.reserve(registry_.max_line_length);
    index_lock_.lock_reader();
    for (sparsetable<PostLine,48>::nonempty_iterator it=index_.nonempty_begin(),ite=index_.nonempty_end();it!=ite;++it){
        it->decode(line);
        line_cursor=line.begin();
        while(line_cursor!=line.end()){
          doc_type=*line_cursor;
          doc_length=*(line_cursor+1);
          hash_hist[doc_type][doc_length]++;
          doc_it=line_cursor+2;
          doc_ite=doc_it+doc_length;
          stats_t* doc_type_gaps = &gaps_hist[doc_type];
          while(doc_it!=doc_ite){
            (*doc_type_gaps)[*doc_it]++;
            doc_it++;
          }
          line_cursor=doc_ite;
      }
    }
    index_lock_.unlock();
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
  
  void Posting::wait(){
    for (uint32_t i=0;i<registry_.slot_count;i++){
      if (slots_[i]->getTaskCount()!=0){
        kc::Thread::sleep(0.2);       
      }
    }
    cout << "Releasing Memory" << endl;
    MallocExtension::instance()->ReleaseFreeMemory();
    cout << "Done!" << endl;
  }
  
  bool Posting::init(){
    // Load the stored docs
    double start = kc::time();
    DocumentCursor* cursor = new DocumentCursor(registry_);
    Document* doc;
    while ((doc=cursor->getNext())!=NULL){
      addDocument(doc);
    }
    delete cursor;
    wait();
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
    if (queue_length>registry_.slot_count*40){
      kc::Thread::sleep(0.05);
      cout << "Sleeping for 0.05 secs with queue length: " << queue_length << endl;
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
      addDocument((*it)->getDocument());
    }
    wait();
    return true;
  }
  
  bool Posting::deleteDocuments(vector<Command*> commands){
    for (vector<Command*>::iterator it=commands.begin(),ite=commands.end();it!=ite;++it){
      deleteDocument((*it)->getDocument());
    }
    wait();
    return true;
  }
  
  bool Posting::isReady(){
    return ready_;
  }
  
  void Posting::fill_status_dictionary(TemplateDictionary* dict){
    dict->SetIntValue("DOC_COUNT",doc_count_);
    dict->SetIntValue("HASH_COUNT",hash_count_);
    dict->SetIntValue("AVERAGE_DOC_LENGTH",(doc_count_>0)?hash_count_/doc_count_:0);
  }
  
  void Posting::fill_list_dictionary(TemplateDictionary* dict,hash_t start){
    uint32_t count=0;
    for (uint32_t i=0;i<slots_.size();i++){
      count+=slots_[i]->fill_list_dictionary(dict,start);
      if (count>=registry_.page_size){
        break;
      }
    }
    TemplateDictionary* page_dict=dict->AddIncludeDictionary("PAGING");
    page_dict->SetFilename(PAGING);
    page_dict->SetValueAndShowSection("PAGE",toString(0),"FIRST");
    if (start>registry_.page_size){
      page_dict->SetValueAndShowSection("PAGE",toString(start-registry_.page_size),"PREVIOUS"); 
    }
    else{
      page_dict->SetValueAndShowSection("PAGE",toString(0),"PREVIOUS"); 
    }
    page_dict->SetValueAndShowSection("PAGE",toString(min(registry_.max_hash_count-registry_.page_size,start+registry_.page_size)),"NEXT");
    page_dict->SetValueAndShowSection("PAGE",toString(registry_.max_hash_count-registry_.page_size),"LAST");
  }

  void Posting::fill_histogram_dictionary(TemplateDictionary* dict){
    histogram_t hash_hist;
    histogram_t gaps_hist;
    for (uint32_t i=0;i<slots_.size();i++){
      slots_[i]->fillHistograms(hash_hist,gaps_hist);
    }
    // Hash histogram
    TemplateDictionary* hash_dict = dict->AddIncludeDictionary("HASH_HISTOGRAM");
    hash_dict->SetFilename(HISTOGRAM);
    hash_dict->SetValue("NAME","hashes");
    hash_dict->SetValue("TITLE","Frequency of documents per hash");
    for (histogram_t::const_iterator it = hash_hist.begin(),ite=hash_hist.end();it!=ite;++it){
      hash_dict->SetValueAndShowSection("DOC_TYPE",toString(it->first),"COLUMNS");
    }
    for (uint32_t i=0;i<500;i++){
      TemplateDictionary* row_dict=hash_dict->AddSectionDictionary("ROW");
      row_dict->SetIntValue("INDEX",i);
      for (histogram_t::iterator it = hash_hist.begin(),ite=hash_hist.end();it!=ite;++it){
          row_dict->SetValueAndShowSection("DOC_COUNTS",toString(it->second[i]),"COLUMN");  
      } 
    }
    // Deltas Histogram
    TemplateDictionary* deltas_dict = dict->AddIncludeDictionary("DELTAS_HISTOGRAM");
    deltas_dict->SetFilename(HISTOGRAM);
    deltas_dict->SetValue("NAME","deltas");
    deltas_dict->SetValue("TITLE","Frequency of document deltas");
    for (histogram_t::const_iterator it = gaps_hist.begin(),ite=gaps_hist.end();it!=ite;++it){
      deltas_dict->SetValueAndShowSection("DOC_TYPE",toString(it->first),"COLUMNS");
    }
    vector<uint32_t> sorted_keys;
    for (histogram_t::iterator it = gaps_hist.begin(),ite=gaps_hist.end();it!=ite;++it){
      for (stats_t::const_iterator it2=it->second.begin(),ite2=it->second.end();it2!=ite2;++it2){
        sorted_keys.push_back(it2->first);
      }
    }
    std::sort(sorted_keys.begin(),sorted_keys.end());
    vector<uint32_t>::iterator res_it=std::unique(sorted_keys.begin(),sorted_keys.end());
    sorted_keys.resize(res_it-sorted_keys.begin());
    for (vector<uint32_t>::const_iterator it=sorted_keys.begin(),ite=sorted_keys.end();it!=ite;++it){
      TemplateDictionary* row_dict=deltas_dict->AddSectionDictionary("ROW");
      row_dict->SetIntValue("INDEX",*it);
      for (histogram_t::iterator it2 = gaps_hist.begin(),ite2=gaps_hist.end();it2!=ite2;++it2){
        row_dict->SetValueAndShowSection("DOC_COUNTS",toString(it2->second[*it]),"COLUMN"); 
      }
    } 
    fill_status_dictionary(dict);
  }
}