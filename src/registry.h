#ifndef _SFMREGISTRY_H                       // duplication check
#define _SFMREGISTRY_H

#include <common.h>
#include <kcplantdb.h>
#include <kccompress.h>
#include <logger.h>
#include <instrumentation.h>

using namespace std;
using namespace kyototycoon;

namespace superfastmatch{

  //Forward Declarations
  class Posting;
  class Document;
  class DocumentManager;
  class AssociationManager;
  class QueueManager;

  // Interface class for easy mocking in tests
  class Registry{
  public:
    virtual bool isClosing()=0;
    // Settings
    virtual uint32_t getHashWidth() const=0;
    virtual uint32_t getHashMask() const=0;
    virtual uint32_t getWindowSize() const=0;
    virtual uint32_t getPostingWindowSize() const=0;
    virtual uint32_t getWhiteSpaceThreshold() const=0;
    virtual uint32_t getWhiteSpaceHash(bool posting=true) const=0;
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
    virtual string getPublicPath() const=0;
    virtual string getAddress() const=0;
    virtual uint32_t getPort() const=0;
    virtual bool isDaemonized() const=0;
    virtual string getLogFile() const=0;

    // Instrumentation
    virtual InstrumentGroupPtr getInstrumentGroup(const int32_t group)=0;

    // DB's
    virtual uint32_t getMode()=0;
    virtual kc::PolyDB* getQueueDB()=0;
    virtual kc::PolyDB* getPayloadDB()=0;
    virtual kc::PolyDB* getDocumentDB()=0;
    virtual kc::PolyDB* getMetaDB()=0;
    virtual kc::PolyDB* getOrderedMetaDB()=0;
    virtual kc::PolyDB* getAssociationDB()=0;
    virtual kc::PolyDB* getMiscDB()=0;

    // Common bits - these might not belong here!
    virtual TemplateCache* getTemplateCache()=0;
    virtual Logger* getLogger()=0;
    virtual Posting* getPostings()=0;
    virtual DocumentManager* getDocumentManager()=0;
    virtual AssociationManager* getAssociationManager()=0;
    virtual QueueManager* getQueueManager()=0;
    
    // Display
    virtual void fillPerformanceDictionary(TemplateDictionary* dict)=0;
    virtual void fillStatusDictionary(TemplateDictionary* dict)=0;
  };

  class FlagsRegistry : public Registry
  {
  private:
    kc::PolyDB* queueDB_;
    kc::PolyDB* payloadDB_;
    kc::PolyDB* documentDB_;
    kc::PolyDB* metaDB_;
    kc::PolyDB* orderedMetaDB_;
    kc::PolyDB* associationDB_;
    kc::PolyDB* miscDB_;
    TemplateCache* templates_;
    Logger* logger_;
    Posting* postings_;
    DocumentManager* documentManager_;
    AssociationManager* associationManager_;
    QueueManager* queueManager_;
    vector<InstrumentGroupPtr> instruments_;
    bool isClosing_;

  public:
    FlagsRegistry();
    ~FlagsRegistry();
    
    bool isClosing();
    void close();
        
    enum InstrumentGroups{
      WORKER,
      QUEUE,
      INDEX
    };
    
    uint32_t getHashWidth() const;
    uint32_t getHashMask() const;
    uint32_t getWindowSize() const;
    uint32_t getPostingWindowSize() const;
    uint32_t getWhiteSpaceThreshold() const;
    uint32_t getWhiteSpaceHash(bool masked=true) const;
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
    string getPublicPath() const;
    string getAddress() const;
    uint32_t getPort() const;
    double getTimeout() const;
    bool isDaemonized() const;
    string getLogFile() const;
    InstrumentGroupPtr getInstrumentGroup(const int32_t group);

    uint32_t getMode();
    kc::PolyDB* getQueueDB();
    kc::PolyDB* getPayloadDB();
    kc::PolyDB* getDocumentDB();
    kc::PolyDB* getMetaDB();
    kc::PolyDB* getOrderedMetaDB();
    kc::PolyDB* getAssociationDB();
    kc::PolyDB* getMiscDB();
    
    TemplateCache* getTemplateCache();
    Logger* getLogger();
    Posting* getPostings();
    DocumentManager* getDocumentManager();
    AssociationManager* getAssociationManager();
    QueueManager* getQueueManager();

    void fillStatusDictionary(TemplateDictionary* dict);
    void fillPerformanceDictionary(TemplateDictionary* dict);
      
  private:
    bool openDatabases();
    void fillDbDictionary(TemplateDictionary* dict, kc::PolyDB* db, const string name);
  };
}
#endif
