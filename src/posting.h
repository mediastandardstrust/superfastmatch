#ifndef _SFMPOSTING_H                       // duplication check
#define _SFMPOSTING_H

#include <google/sparsetable>
#include <boost/pool/pool_alloc.hpp>
#include <common.h>
#include <instrumentation.h>
#include <templates.h>
#include <postline.h>
#include <query.h>
#include <task.h>
#include <search.h>

using google::sparsetable;

namespace superfastmatch
{
  // Forward Declarations
  class Registry;
  class Command;
  class Association;

  //Typedefs
  typedef sparsetable<unsigned char*,48> index_t;
  typedef unordered_map<uint32_t,uint64_t> stats_t;
  typedef unordered_map<uint32_t,stats_t> histogram_t;

  class PostingSlot{
  private:
    Registry* registry_;
    const uint32_t slot_number_;
    const uint32_t offset_;
    const uint32_t span_;
    index_t index_;
    kc::RWLock index_lock_;
    PostingTaskQueue queue_;
    
  public:
    PostingSlot(Registry* registry,uint32_t slot_number);
    ~PostingSlot();
    
    bool alterIndex(DocumentPtr doc,TaskPayload::TaskOperation operation);
    bool searchIndex(const uint32_t doctype,const uint32_t docid,const uint32_t hash, const uint32_t position,PostLine& line,search_t& results);

    uint64_t addTask(TaskPayload* payload);
    uint64_t getTaskCount();
    void finishTasks();
    size_t getHashCount();
    void lockSlotForReading();
    void unlockSlotForReading();
    
    uint32_t fillListDictionary(TemplateDictionary* dict,uint32_t start);
    void fillHistograms(histogram_t& hash_hist,histogram_t& gaps_hist);
  };

  class Posting : public Instrumented<Posting>{
  private:    
    Registry* registry_;
    vector<PostingSlot*> slots_;
    uint64_t doc_count_;
    uint64_t total_doc_length_;
    bool ready_;
        
  public:
    Posting(Registry* registry);
    ~Posting();
    
    bool init();
    size_t getHashCount();
    void finishTasks();
    void searchIndex(Search& search);
    // Following three methods return the current queue length for all slots combined
    uint64_t alterIndex(DocumentPtr doc,TaskPayload::TaskOperation operation);
    uint64_t addDocument(DocumentPtr doc);
    uint64_t deleteDocument(DocumentPtr doc);
    bool isReady();

    void fillStatusDictionary(TemplateDictionary* dict);
    void fillListDictionary(TemplateDictionary* dict,uint32_t start);
    void fillHistogramDictionary(TemplateDictionary* dict);
  private:
    void lockSlotsForReading();
    void unlockSlotsForReading();
    DISALLOW_COPY_AND_ASSIGN(Posting);
  };  
}

#endif
