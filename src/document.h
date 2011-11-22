#ifndef _SFMDOCUMENT_H                       // duplication check
#define _SFMDOCUMENT_H

#include <common.h>
#include <instrumentation.h>
#include <registry.h>
#include <association.h>
#include <posting.h>

namespace superfastmatch
{ 
  typedef vector<uint32_t> hashes_vector;
  typedef bitset<(1<<26)> hashes_bloom;
  typedef map<std::string,std::string> metadata_map;
  
  class DocumentManager; // Forward Declaration
  
  struct MetaKeyComparator {
    bool operator() (const string& lhs, const string& rhs) const{
      if (lhs==rhs)
        return false;
      if (lhs=="title")
        return true;
      if ((lhs=="characters") && (rhs=="title"))
        return false;
      if (lhs=="characters")
        return true;
      return lhs<rhs;
    }
  };
  
  class Document : public enable_shared_from_this<Document>, public Instrumented<Document>
  {
  friend class DocumentManager;
  private:
    uint32_t doctype_;
    uint32_t docid_;
    bool permanent_;
    string* empty_meta_;
    Registry* registry_;
    string* key_;
    string* text_;
    metadata_map* metadata_;
    hashes_vector* hashes_;
    hashes_vector* posting_hashes_;
    hashes_bloom* bloom_;

  public:
    ~Document();
    hashes_vector& getHashes();
    hashes_vector& getPostingHashes();
    hashes_bloom& getBloom();
    bool isPermanent();
    string& getMeta(const string& key);
    bool setMeta(const string& key, const string& value);
    bool getMetaKeys(vector<string>& keys);
    string& getText();
    string getCleanText();
    string getCleanText(const uint32_t position,const uint32_t length);
    string& getKey();
    uint32_t doctype();
    uint32_t docid();
    
    void fillDocumentDictionary(TemplateDictionary* dict);
    
    friend std::ostream& operator<< (std::ostream& stream, Document& document);
    friend bool operator< (Document& lhs,Document& rhs);
    
  private:
    explicit Document(const uint32_t doctype,const uint32_t docid,const bool permanent,Registry* registry);
    explicit Document(const string& key,const bool permanent,Registry* registry);
    bool remove();
    bool setText(const string& text);
    bool initMeta();
    bool initText();
    bool initCleanText();
    bool initHashes();
    bool initPostingHashes();
    bool initBloom();
  };
  
  class DocumentManager
  {
  public:
    enum DocumentState{
      NONE            = 0,
      META            = 1 << 0,
      TEXT            = 1 << 1,
      HASHES          = 1 << 2,   // HASHES depends on TEXT
      POSTING_HASHES  = 1 << 3,   // POSTING_HASHES depends on TEXT
      BLOOM           = 1 << 4    // BLOOM depends on HASHES
    };
  private:
    Registry* registry_;
    static const int32_t DEFAULT_STATE = META|TEXT|HASHES|POSTING_HASHES|BLOOM;
  public:
    explicit DocumentManager(Registry* registry);
    ~DocumentManager();
    
    DocumentPtr createTemporaryDocument(const string& content,const int32_t state=DEFAULT_STATE);
    DocumentPtr createPermanentDocument(const uint32_t doctype, const uint32_t docid,const string& content,const int32_t state=DEFAULT_STATE);
    bool removePermanentDocument(DocumentPtr doc);
    DocumentPtr getDocument(const uint32_t doctype, const uint32_t docid,const int32_t state=DEFAULT_STATE);
    DocumentPtr getDocument(const string& key,const int32_t state=DEFAULT_STATE);
    
  private:
    bool initDoc(const DocumentPtr doc,const int32_t state);
    DocumentPtr createDocument(const uint32_t doctype, const uint32_t docid,const string& content,const int32_t state,const bool commit);
    DISALLOW_COPY_AND_ASSIGN(DocumentManager);
  };
}//namespace Superfastmatch

#endif                                   // duplication check
