#ifndef _SFMDOCUMENT_H                       // duplication check
#define _SFMDOCUMENT_H

#include <vector>
#include <bitset>
#include <map>
#include <algorithm>
#include <string>
#include <cctype>
#include <tr1/memory>
#include <common.h>
#include <registry.h>
#include <association.h>


namespace superfastmatch
{   
  class DocumentCursor
  {
  private:
    Registry* registry_;
    kc::PolyDB::Cursor* cursor_;
    
  public:
    DocumentCursor(Registry* registry);
    ~DocumentCursor();
    
    bool jumpFirst();
    bool jumpLast();
    bool jump(string& key);
    Document* getNext();
    Document* getPrevious();
    uint32_t getCount();
    
    void fill_list_dictionary(TemplateDictionary* dict,uint32_t doctype,uint32_t docid);
  };
  
  class DocumentManager; // Forward Declaration
  
  class Document
  {
  friend class DocumentManager;
  public:
    typedef std::vector<hash_t> hashes_vector;
    typedef std::bitset<(1<<24)> hashes_bloom;
    typedef std::map<std::string,std::string> metadata_map;
    
  private:
    uint32_t doctype_;
    uint32_t docid_;
    string* empty_meta_;
    Registry* registry_;
    string* key_;
    string* text_;
    string* clean_text_;
    metadata_map* metadata_;
    hashes_vector* hashes_;
    hashes_bloom* bloom_;
  
  public:
    Document(const uint32_t doctype,const uint32_t docid,const string& content,Registry* registry);
    Document(const uint32_t doctype,const uint32_t docid,Registry* registry);
    Document(string& key,Registry* registry);
    
    ~Document();
    
    // Returns false if document already exists
    bool load();
    bool save();
    bool remove();
    
    hashes_vector& hashes();
    hashes_bloom& bloom();
    string& getMeta(const char* key);
    bool setMeta(const char* key, const char* value);
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
    bool initMeta();
    bool initText();
    bool initCleanText();
    bool initHashes();
    bool initBloom();
  };
  

  typedef std::tr1::shared_ptr<Document> DocumentPtr;
  class DocumentManager
  {
  public:
    enum DocumentState{
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
    DocumentManager(Registry* registry);
    ~DocumentManager();
    
    DocumentPtr createTemporaryDocument(const string& content,const int32_t state=DEFAULT_STATE);
    DocumentPtr createPermanentDocument(const uint32_t doctype, const uint32_t docid,const string& content,const int32_t state=DEFAULT_STATE);
    DocumentPtr getDocument(const uint32_t doctype, const uint32_t docid,const int32_t state=DEFAULT_STATE);
    
  private:
    bool initDoc(const DocumentPtr doc,const int32_t state);
    DISALLOW_COPY_AND_ASSIGN(DocumentManager);
  };
  
}//namespace Superfastmatch

#endif                                   // duplication check
