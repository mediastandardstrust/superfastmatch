#ifndef _SFMDOCUMENT_H                       // duplication check
#define _SFMDOCUMENT_H

#include <vector>
#include <bitset>
#include <map>
#include <algorithm>
#include <string>
#include <cctype>
#include <common.h>
#include <registry.h>
#include <association.h>
#include <posting.h>

namespace superfastmatch
{ 
  typedef std::vector<hash_t> hashes_vector;
  typedef std::bitset<(1<<24)> hashes_bloom;
  typedef std::map<std::string,std::string> metadata_map;
  
  class DocumentManager; // Forward Declaration
  
  class Document : public std::tr1::enable_shared_from_this<Document>
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
    string* clean_text_;
    metadata_map* metadata_;
    hashes_vector* hashes_;
    hashes_bloom* bloom_;

  public:
    enum DocumentOrder{
      DEFAULT,
      FORWARD,
      REVERSE
    };
    ~Document();
    hashes_vector& getHashes();
    hashes_bloom& getBloom();
    string& getMeta(const string& key);
    bool setMeta(const string& key, const string& value);
    bool getMetaKeys(vector<string>& keys);
    string& getText();
    string& getCleanText();
    string& getKey();
    const uint32_t doctype();
    const uint32_t docid();
    
    void fill_document_dictionary(TemplateDictionary* dict);
    
    friend std::ostream& operator<< (std::ostream& stream, Document& document);
    friend bool operator< (Document& lhs,Document& rhs);
    
  private:
    explicit Document(const uint32_t doctype,const uint32_t docid,const bool permanent,Registry* registry);
    explicit Document(const string& key,const bool permanent,Registry* registry);
    string generateMetaKey(const DocumentOrder order,const string& meta,const string& value);
    bool remove();
    bool setText(const string& text);
    bool initMeta();
    bool initText();
    bool initCleanText();
    bool initHashes();
    bool initBloom();
  };
  
  class DocumentManager
  {
  public:
    enum DocumentState{
      NONE        = 0,
      META        = 1 << 0,
      TEXT        = 1 << 1,
      CLEAN_TEXT  = 1 << 2,   // CLEAN_TEXT depends on TEXT 
      HASHES      = 1 << 3,   // HASHES depends on CLEAN_TEXT
      BLOOM       = 1 << 4    // BLOOM depends on HASHES
    };
  private:
    Registry* registry_;
    static const int32_t DEFAULT_STATE = META|TEXT|CLEAN_TEXT|HASHES|BLOOM;
  public:
    explicit DocumentManager(Registry* registry);
    ~DocumentManager();
    
    DocumentPtr createTemporaryDocument(const string& content,const int32_t state=DEFAULT_STATE);
    DocumentPtr createPermanentDocument(const uint32_t doctype, const uint32_t docid,const string& content,const int32_t state=DEFAULT_STATE);
    bool removePermanentDocument(DocumentPtr doc);
    DocumentPtr getDocument(const uint32_t doctype, const uint32_t docid,const int32_t state=DEFAULT_STATE);
    DocumentPtr getDocument(const string& key,const int32_t state=DEFAULT_STATE);
    vector<DocumentPtr> getDocuments(const uint32_t doctype=0,const int32_t state=DEFAULT_STATE);
    bool associateDocument(DocumentPtr doc);
    
  private:
    void initDoc(const DocumentPtr doc,const int32_t state);
    DocumentPtr createDocument(const uint32_t doctype, const uint32_t docid,const string& content,const int32_t state,const bool commit);
    DISALLOW_COPY_AND_ASSIGN(DocumentManager);
  };
  
  class DocumentCursor
  {
  private:
    Registry* registry_;
    const Document::DocumentOrder order_;
    const string meta_key_;
    string previous_key_;
    kc::PolyDB::Cursor* cursor_;
    
  public:
    DocumentCursor(Registry* registry, const string& meta_key="",const Document::DocumentOrder order=Document::DEFAULT);
    ~DocumentCursor();
    
    bool jumpFirst();
    bool jumpLast();
    bool jump(string& key);
    DocumentPtr getNext(const int32_t state=DocumentManager::META);
    DocumentPtr getPrevious();
    uint32_t getCount();
    
    void fill_list_dictionary(TemplateDictionary* dict,uint32_t doctype,uint32_t docid);
  };
  
}//namespace Superfastmatch

#endif                                   // duplication check
