#ifndef _SFMPOSTING_H                       // duplication check
#define _SFMPOSTING_H

// #include <cassert>
// #include <deque>
// #include <algorithm>
// #include <map>
#include <google/sparsetable>
#include <common.h>
#include <templates.h>
#include <postline.h>
#include <query.h>
#include <task.h>

using google::sparsetable;
using namespace std;

namespace superfastmatch
{
  // Forward Declarations
  class Registry;
  class Command;
  class Association;
    
  struct DocTally{
    uint64_t count;
    uint64_t total;
    uint32_t previous_1;
    uint32_t previous_2;
    uint32_t previous_3;
    uint32_t previous_4;
    uint32_t previous_5;
    uint32_t previous_6;
    
    DocTally():
    count(0),
    total(0),
    previous_1(0xfffffff0),
    previous_2(0xfffffff0),
    previous_3(0xfffffff0),
    previous_4(0xfffffff0),
    previous_5(0xfffffff0),
    previous_6(0xfffffff0)
    {}
    
    double getScore() const{
      return (count*count)/double(total);
    }
  };
  
  typedef struct
  {
    bool operator()(const DocTally &lhs, const DocTally &rhs) const { 
      return lhs.count>rhs.count;
      // return lhs.getScore()>rhs.getScore();
    }
  } DocTallyEq;
    
  typedef sparsetable<unsigned char*,48> index_t;
  typedef unordered_map<uint32_t,uint64_t> stats_t;
  typedef unordered_map<uint32_t,stats_t> histogram_t;
  typedef unordered_map<DocPair,DocTally,DocPairHash,DocPairEq> search_t;
  typedef multimap<DocTally,DocPair,DocTallyEq> inverted_search_t;

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

    uint32_t fill_list_dictionary(TemplateDictionary* dict,uint32_t start);
    void fillHistograms(histogram_t& hash_hist,histogram_t& gaps_hist);
  };

  class Posting{
  private:    
    Registry* registry_;
    vector<PostingSlot*> slots_;
    uint32_t doc_count_;
    uint64_t total_doc_length_;
    bool ready_;
        
  public:
    Posting(Registry* registry);
    ~Posting();
    
    bool init();
    size_t getHashCount();
    void finishTasks();
    void searchIndex(DocumentPtr doc,search_t& results,inverted_search_t& pruned_results);
    // Following three methods return the current queue length for all slots combined   
    uint64_t alterIndex(DocumentPtr doc,TaskPayload::TaskOperation operation);
    uint64_t addDocument(DocumentPtr doc);
    uint64_t deleteDocument(DocumentPtr doc);
    bool isReady();

    void fill_status_dictionary(TemplateDictionary* dict);
    void fill_list_dictionary(TemplateDictionary* dict,uint32_t start);
    void fill_histogram_dictionary(TemplateDictionary* dict);
  };
}

#endif
