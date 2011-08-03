#include "posting.h"

//Is this naughty?
#include "document.h"
#include "command.h"
#include "registry.h"
#include "association.h"
#include <google/profiler.h>

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

  // -------------------
  // PostingSlot members
  // -------------------
  
  PostingSlot::PostingSlot(Registry* registry,uint32_t slot_number):
  registry_(registry),slot_number_(slot_number),
  offset_((registry->getMaxHashCount()/registry->getSlotCount())*slot_number), // Offset for sparsetable insertion
  span_(registry->getMaxHashCount()/registry->getSlotCount()), // Span of slot, ie. ignore every hash where (hash-offset)>span
  codec_(new VarIntCodec()),
  line_(codec_,registry->getMaxLineLength()),
  index_(registry->getMaxHashCount()/registry->getSlotCount())
  {
    // 1 thread per slot, increase slot_number to get more threads!
    queue_.start(1);
  }
  
  PostingSlot::~PostingSlot(){
    queue_.finish();
    index_.clear();
  }

  bool PostingSlot::alterIndex(Document* doc,TaskPayload::TaskOperation operation){
    index_lock_.lock_writer();
    hash_t hash;
    hash_t hash_mask=registry_->getHashMask();
    uint32_t hash_width=registry_->getHashWidth();
    size_t incoming_length;
    size_t outgoing_length;
    // Where hash width is below 32 we will get duplicates per document
    // We discard them with a no operation 
    for (Document::hashes_vector::const_iterator it=doc->unique_sorted_hashes().begin(),ite=doc->unique_sorted_hashes().end();it!=ite;++it){
      hash = ((*it>>hash_width)^(*it&hash_mask))-offset_;
      uint32_t doctype=doc->doctype();
      uint32_t docid=doc->docid();
      if (hash<span_){
        unsigned char* entry=NULL;
        bool noop=false;
        if (!index_.test(hash)){
          entry = new unsigned char[8];
          memset(entry,0,8);
          index_.set(hash,entry);
        }
        entry=index_[hash];
        line_.load(entry);
        incoming_length=line_.getLength();
        switch (operation){
          case TaskPayload::AddDocument:
            noop=not line_.addDocument(doctype,docid);
            break;
          case TaskPayload::DeleteDocument:
            noop=not line_.deleteDocument(doctype,docid);
            break;
        }
        if (!noop){
          outgoing_length=line_.getLength();
          if ((outgoing_length/8)>(incoming_length/8)){
            size_t size=((outgoing_length/8)+1)*8;
            delete[] entry;
            entry = new unsigned char[size];
            index_.set(hash,entry);
          }
          line_.commit(entry);
        }
      }
    }
    index_lock_.unlock();
    return true;
  }
  
  bool PostingSlot::searchIndex(Document* doc,search_t& results){
    hash_t hash;
    hash_t hash_mask=registry_->getHashMask();
    uint32_t hash_width=registry_->getHashWidth();
    vector<uint32_t> docids;
    vector<uint32_t> doctypes;
    DocTally* tally;
    DocPair pair(0,0);
    size_t position=0;
    size_t doc_count;
    size_t max_distance=registry_->getMaxDistance();
    index_lock_.lock_reader();
    for (vector<hash_t>::const_iterator it=doc->hashes().begin(),ite=doc->hashes().end();it!=ite;++it){
      hash = ((*it>>hash_width)^(*it&hash_mask))-offset_;
      if ((hash<span_) && (index_.test(hash))){
        line_.load(index_.unsafe_get(hash));  
        line_.getDocTypes(doctypes);
        for (vector<uint32_t>::const_iterator it2=doctypes.begin(),ite2=doctypes.end();it2!=ite2;++it2){
          line_.getDocIds(*it2,docids);
          doc_count=docids.size();
          for (vector<uint32_t>::const_iterator it3=docids.begin(),ite3=docids.end();it3!=ite3;++it3){
            pair.doc_type=*it2;
            pair.doc_id=*it3;
            tally=&results[pair];
            // cout << results.bucket_count() << ":" << results.load_factor() << ":" << results.size() <<endl; 
            if ((position-tally->last_seen)<max_distance){
              tally->count++;
              tally->total+=doc_count;
            }
            tally->last_seen=position;
          }
        }
      }
      position++;
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
    index_lock_.lock_reader();
    uint32_t count=0;
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
    vector<uint32_t> doc_types;
    vector<uint32_t> doc_ids;
    index_t::nonempty_iterator it=index_.get_iter(hash);
    while (it!=index_.nonempty_end() && count<registry_->getPageSize()){
      count++;
      line_.load(*it);
      line_.getDocTypes(doc_types);
      TemplateDictionary* posting_dict=dict->AddSectionDictionary("POSTING");
      TemplateDictionary* hash_dict=posting_dict->AddSectionDictionary("HASH");
      hash_dict->SetIntValue("HASH",index_.get_pos(it)+offset_);
      hash_dict->SetIntValue("BYTES",line_.getLength());
      hash_dict->SetIntValue("DOC_TYPE_COUNT",doc_types.size());
      for (size_t i=0;i<doc_types.size();i++){
        posting_dict->SetIntValue("DOC_TYPE",doc_types[i]);
        line_.getDocIds(doc_types[i],doc_ids);
        uint32_t previous=0;
        for (size_t j=0;j<doc_ids.size();j++){
          TemplateDictionary* doc_id_dict = posting_dict->AddSectionDictionary("DOC_IDS");
          TemplateDictionary* doc_delta_dict = posting_dict->AddSectionDictionary("DOC_DELTAS");
          doc_id_dict->SetIntValue("DOC_ID",doc_ids[j]); 
          doc_delta_dict->SetIntValue("DOC_DELTA",doc_ids[j]-previous);
          previous=doc_ids[j];
        }
        if (i!=(doc_types.size()-1)){
          posting_dict=dict->AddSectionDictionary("POSTING");
        }
      }
      it++;
    }
    index_lock_.unlock();
    return count;
  }
  
  void PostingSlot::fillHistograms(histogram_t& hash_hist,histogram_t& gaps_hist){
    index_lock_.lock_reader();
    vector<uint32_t> doc_types;
    vector<uint32_t> doc_deltas;
    uint32_t doc_type;
    stats_t* doc_type_gaps;
    for (index_t::nonempty_iterator it=index_.nonempty_begin(),ite=index_.nonempty_end();it!=ite;++it){
      line_.load(*it);
      line_.getDocTypes(doc_types);
      for(size_t i=0;i<doc_types.size();i++){
        doc_type=doc_types[i];
        hash_hist[doc_type][line_.getLength(doc_type)]++;
        line_.getDeltas(doc_type,doc_deltas);
        doc_type_gaps=&gaps_hist[doc_type];
        for(size_t j=0;j<doc_deltas.size();j++){
          (*doc_type_gaps)[doc_deltas[j]]++;
        }
      }
    }
    index_lock_.unlock();
  }
  
  // ---------------
  // Posting members
  // ---------------
  
  Posting::Posting(Registry* registry):
  registry_(registry),doc_count_(0),hash_count_(0),ready_(false)
  {
    for (uint32_t i=0;i<registry->getSlotCount();i++){
      slots_.push_back(new PostingSlot(registry,i));
    }
  }
  
  Posting::~Posting(){
    for (size_t i=0;i<slots_.size();i++){
      delete slots_[i];
    }
  }
  
  void Posting::wait(){
    for (size_t i=0;i<slots_.size();i++){
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
    TaskPayload* task = new TaskPayload(doc,operation,slots_.size());
    for (size_t i=0;i<slots_.size();i++){
      queue_length+=slots_[i]->addTask(task);
    }
    if (queue_length>slots_.size()*40){
      kc::Thread::sleep(0.05);
      cout << "Sleeping for 0.05 secs with queue length: " << queue_length << endl;
    }
    return queue_length;
  }
  
  void Posting::searchIndex(Document* doc,search_t& results,inverted_search_t& pruned_results){
    for (size_t i=0;i<slots_.size();i++){
      slots_[i]->searchIndex(doc,results);
    }
    for (search_t::iterator it=results.begin(),ite=results.end();it!=ite;it++){
      if ((it->second.count>1) && !(it->first.doc_type==doc->doctype() && it->first.doc_id==doc->docid())){
        pruned_results.insert(pair<DocTally,DocPair>(it->second,it->first));
      }
    }
  }
  
  uint64_t Posting::addDocument(Document* doc){
    cout << "Adding: " << *doc << endl;
    hash_count_+=doc->unique_sorted_hashes().size();
    doc_count_++;
    uint64_t queue_length=alterIndex(doc,TaskPayload::AddDocument);
    return queue_length;
  }
  
  uint64_t Posting::deleteDocument(Document* doc){
    cout << "Deleting: " << *doc << endl;
    hash_count_-=doc->unique_sorted_hashes().size();
    doc_count_--;
    uint64_t queue_length=alterIndex(doc,TaskPayload::DeleteDocument);
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
  
  bool Posting::addAssociations(vector<Command*> commands){
    ProfilerStart("/tmp/superfastmatch");
    search_t results;
    inverted_search_t pruned_results;
    Document* doc;
    size_t num_results=registry_->getNumResults();
    size_t count;
    for (vector<Command*>::iterator it=commands.begin(),ite=commands.end();it!=ite;++it){
      count=0;
      results.clear();
      pruned_results.clear();
      doc=(*it)->getDocument();
      cout << "Associating: " << *doc << endl;
      searchIndex(doc,results,pruned_results);
      for(inverted_search_t::iterator it2=pruned_results.begin(),ite2=pruned_results.end();it2!=ite2 && count<num_results;++it2){
        Document other(it2->second.doc_type,it2->second.doc_id,registry_);
        Association association(registry_,doc,&other);
        association.save();
        count++;
      } 
      delete doc;
    }
    ProfilerStop();
    return true;
  }
  
  bool Posting::isReady(){
    return ready_;
  }
  
  void Posting::fill_search_dictionary(Document* doc,TemplateDictionary* dict){
    search_t results;
    inverted_search_t pruned_results;
    searchIndex(doc,results,pruned_results);
    size_t count=0;
    size_t num_results=registry_->getNumResults();
    inverted_search_t::iterator it=pruned_results.begin();
    TemplateDictionary* association_dict=dict->AddIncludeDictionary("ASSOCIATION");
    association_dict->SetFilename(ASSOCIATION);
    while(it!=pruned_results.end() && count<num_results){
      TemplateDictionary* result_dict=dict->AddSectionDictionary("RESULT");
      result_dict->SetIntValue("DOC_TYPE",it->second.doc_type);
      result_dict->SetIntValue("DOC_ID",it->second.doc_id);
      result_dict->SetIntValue("COUNT",it->first.count);
      result_dict->SetIntValue("TOTAL",it->first.total);
      result_dict->SetFormattedValue("HEAT","%.2f",double(it->first.total)/it->first.count);
      Document other(it->second.doc_type,it->second.doc_id,registry_);
      Association association(registry_,doc,&other);
      association.fill_item_dictionary(association_dict);
      count++;
      it++;
    }
  }
  
  void Posting::fill_status_dictionary(TemplateDictionary* dict){
    dict->SetIntValue("HASH_WIDTH",registry_->getHashWidth());
    dict->SetIntValue("SLOT_COUNT",registry_->getSlotCount());
    dict->SetIntValue("DOC_COUNT",doc_count_);
    dict->SetIntValue("HASH_COUNT",hash_count_);
    dict->SetIntValue("AVERAGE_DOC_LENGTH",(doc_count_>0)?hash_count_/doc_count_:0);
  }
  
  void Posting::fill_list_dictionary(TemplateDictionary* dict,hash_t start){
    uint32_t count=0;
    for (size_t i=0;i<slots_.size();i++){
      count+=slots_[i]->fill_list_dictionary(dict,start);
      if (count>=registry_->getPageSize()){
        break;
      }
    }
    TemplateDictionary* page_dict=dict->AddIncludeDictionary("PAGING");
    page_dict->SetFilename(PAGING);
    page_dict->SetValueAndShowSection("PAGE",toString(0),"FIRST");
    if (start>registry_->getPageSize()){
      page_dict->SetValueAndShowSection("PAGE",toString(start-registry_->getPageSize()),"PREVIOUS"); 
    }
    else{
      page_dict->SetValueAndShowSection("PAGE",toString(0),"PREVIOUS"); 
    }
    page_dict->SetValueAndShowSection("PAGE",toString(min(registry_->getMaxHashCount()-registry_->getPageSize(),start+registry_->getPageSize())),"NEXT");
    page_dict->SetValueAndShowSection("PAGE",toString(registry_->getMaxHashCount()-registry_->getPageSize()),"LAST");
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