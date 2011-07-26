#ifndef _SFMREGISTRY_H                       // duplication check
#define _SFMREGISTRY_H

#include <map>
#include <common.h>
#include <kcplantdb.h>
#include <kccompress.h>
#include <posting.h>
#include <logger.h>

using namespace std;
using namespace kyototycoon;

namespace superfastmatch{

  // Abstract class for easy mocking in tests
  class RegistryInterface{
  public:
    // Settings
    virtual uint32_t getHashWidth();
    virtual uint32_t getHashMask();
    virtual uint32_t getWindowSize();
    virtual uint32_t getThreadCount();
    virtual uint32_t getSlotCount();
    virtual size_t getPageSize();
    virtual size_t getNumResults();
    virtual size_t getMaxLineLength();
    virtual size_t getMaxHashCount();
    virtual size_t getMaxBatchCount();
    virtual size_t getMaxDistance();
    virtual double getTimeout();

    // DB's
    virtual kc::ForestDB* getQueueDB();
    virtual kc::ForestDB* getDocumentDB();
    virtual kc::ForestDB* getMetaDB();
    virtual kc::ForestDB* getHashesDB();
    virtual kc::PolyDB* getAssociationDB();
    virtual kc::PolyDB* getMiscDB();

    // Common bits - these might not belong here!
    virtual TemplateCache* getTemplates();
    virtual Logger* getLogger();
    virtual Posting* getPostings();    
  };

  class Registry : RegistryInterface
  {
  private:
    kc::Compressor* comp_;
  public:
    uint32_t getHashWidth();
    uint32_t window_size;
    uint32_t hash_mask;
    uint32_t thread_count;
    uint32_t slot_count; // Must be either 2,4,8,16,etc.
    uint64_t page_size;
    uint64_t num_results;
    uint32_t max_line_length; //Needs to be aware of max stack size for platform
    uint64_t max_hash_count;
    uint32_t max_batch_count;
    uint64_t max_distance;
    double timeout;
    kc::ForestDB* queueDB;
    kc::ForestDB* documentDB;
    kc::ForestDB* hashesDB;
    kc::PolyDB* associationDB;
    kc::PolyDB* miscDB;
    TemplateCache* templates;
    Logger* logger;
    // TODO rename to index 
    Posting* postings;
    
    Registry();
    ~Registry();

    void fill_status_dictionary(TemplateDictionary* dict);
      
  private:
    void fill_db_dictionary(TemplateDictionary* dict, kc::ForestDB* db, const string name);
    void status(std::ostream& s, kc::ForestDB* db);
  };
}
#endif
