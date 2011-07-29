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

  // Interface class for easy mocking in tests
  class RegistryInterface{
  public:
    // Settings
    virtual uint32_t getHashWidth() const=0;
    virtual hash_t getHashMask() const=0;
    virtual uint32_t getWindowSize() const=0;
    virtual uint32_t getThreadCount() const=0;
    virtual uint32_t getSlotCount() const=0;
    virtual size_t getPageSize() const=0;
    virtual size_t getNumResults() const=0;
    virtual size_t getMaxLineLength() const=0;
    virtual size_t getMaxHashCount() const=0;
    virtual size_t getMaxBatchCount() const=0;
    virtual size_t getMaxDistance() const=0;
    virtual double getTimeout() const=0;
    virtual string getDataPath() const=0;
    virtual string getAddress() const=0;
    virtual uint32_t getPort() const=0;

    // DB's
    virtual uint32_t getMode()=0;
    virtual kc::ForestDB* getQueueDB()=0;
    virtual kc::ForestDB* getDocumentDB()=0;
    virtual kc::ForestDB* getMetaDB()=0;
    virtual kc::ForestDB* getHashesDB()=0;
    virtual kc::ForestDB* getAssociationDB()=0;
    virtual kc::PolyDB* getMiscDB()=0;

    // Common bits - these might not belong here!
    virtual TemplateCache* getTemplateCache()=0;
    virtual Logger* getLogger()=0;
    virtual Posting* getPostings()=0;    
  };

  class Registry : public RegistryInterface
  {
  private:
    kc::Compressor* comp_;
    kc::ForestDB* queueDB_;
    kc::ForestDB* documentDB_;
    kc::ForestDB* metaDB_;
    kc::ForestDB* hashesDB_;
    kc::ForestDB* associationDB_;
    kc::PolyDB* miscDB_;
    TemplateCache* templates_;
    Logger* logger_;
    Posting* postings_;

  public:
    Registry();
    ~Registry();
    
    uint32_t getHashWidth() const;
    hash_t getHashMask() const;
    uint32_t getWindowSize() const;
    uint32_t getThreadCount() const;
    uint32_t getSlotCount() const;
    size_t getPageSize() const;
    size_t getNumResults() const;
    size_t getMaxLineLength() const;
    size_t getMaxHashCount() const;
    size_t getMaxBatchCount() const;
    size_t getMaxDistance() const;
    string getDataPath() const;
    string getAddress() const;
    uint32_t getPort() const;
    double getTimeout() const;

    uint32_t getMode();
    kc::ForestDB* getQueueDB();
    kc::ForestDB* getDocumentDB();
    kc::ForestDB* getMetaDB();
    kc::ForestDB* getHashesDB();
    kc::ForestDB* getAssociationDB();
    kc::PolyDB* getMiscDB();
    TemplateCache* getTemplateCache();
    Logger* getLogger();
    Posting* getPostings();

    void fill_status_dictionary(TemplateDictionary* dict);
      
  private:
    void fill_db_dictionary(TemplateDictionary* dict, kc::BasicDB* db, const string name);
    void status(std::ostream& s, kc::BasicDB* db);
  };
}
#endif
