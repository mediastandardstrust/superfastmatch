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
  class Registry
  {
  private:
    kc::Compressor* comp_;
  public:
    uint32_t window_size;
    uint32_t hash_width;
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
    
    Registry(const string& filename);
    ~Registry();

    void fill_status_dictionary(TemplateDictionary* dict);
      
  private:
    void fill_db_dictionary(TemplateDictionary* dict, kc::ForestDB* db, const string name);
    void status(std::ostream& s, kc::ForestDB* db);
  };
}
#endif