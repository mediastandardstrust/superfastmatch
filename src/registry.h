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
  class Registry{
  public:
    // Settings
    virtual uint32_t getHashWidth() const=0;
    virtual hash_t getHashMask() const=0;
    virtual uint32_t getWindowSize() const=0;
    virtual uint32_t getWhiteSpaceThreshold() const=0;
    virtual hash_t getWhiteSpaceHash(bool masked=true) const=0;
    virtual uint32_t getThreadCount() const=0;
    virtual uint32_t getSlotCount() const=0;
    virtual size_t getPageSize() const=0;
    virtual size_t getNumResults() const=0;
    virtual size_t getMaxLineLength() const=0;
    virtual size_t getMaxHashCount() const=0;
    virtual size_t getMaxBatchCount() const=0;
    virtual size_t getMaxPostingThreshold() const=0;
    virtual size_t getMaxDistance() const=0;
    virtual double getTimeout() const=0;
    virtual string getDataPath() const=0;
    virtual string getAddress() const=0;
    virtual uint32_t getPort() const=0;

    // DB's
    virtual uint32_t getMode()=0;
    virtual kc::BasicDB* getQueueDB()=0;
    virtual kc::BasicDB* getDocumentDB()=0;
    virtual kc::PolyDB* getMetaDB()=0;
    virtual kc::BasicDB* getAssociationDB()=0;
    virtual kc::BasicDB* getMiscDB()=0;

    // Common bits - these might not belong here!
    virtual TemplateCache* getTemplateCache()=0;
    virtual Logger* getLogger()=0;
    virtual Posting* getPostings()=0;    
    
    virtual void fill_status_dictionary(TemplateDictionary* dict)=0;
  };

  class FlagsRegistry : public Registry
  {
  private:
    kc::Compressor* comp_;
    kc::ForestDB* queueDB_;
    kc::ForestDB* documentDB_;
    kc::PolyDB* metaDB_;
    kc::ForestDB* associationDB_;
    kc::PolyDB* miscDB_;
    TemplateCache* templates_;
    Logger* logger_;
    Posting* postings_;

  public:
    FlagsRegistry();
    ~FlagsRegistry();
    
    uint32_t getHashWidth() const;
    hash_t getHashMask() const;
    uint32_t getWindowSize() const;
    uint32_t getWhiteSpaceThreshold() const;
    hash_t getWhiteSpaceHash(bool masked=true) const;
    uint32_t getThreadCount() const;
    uint32_t getSlotCount() const;
    size_t getPageSize() const;
    size_t getNumResults() const;
    size_t getMaxLineLength() const;
    size_t getMaxHashCount() const;
    size_t getMaxBatchCount() const;
    size_t getMaxPostingThreshold() const;
    size_t getMaxDistance() const;
    string getDataPath() const;
    string getAddress() const;
    uint32_t getPort() const;
    double getTimeout() const;

    uint32_t getMode();
    kc::BasicDB* getQueueDB();
    kc::BasicDB* getDocumentDB();
    kc::PolyDB* getMetaDB();
    kc::BasicDB* getAssociationDB();
    kc::BasicDB* getMiscDB();
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
