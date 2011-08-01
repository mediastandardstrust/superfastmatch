#ifndef _SFMDOCUMENT_H                       // duplication check
#define _SFMDOCUMENT_H

#include <vector>
#include <bitset>
#include <map>
#include <algorithm>
#include <string>
#include <common.h>
#include <registry.h>

namespace superfastmatch
{   
  class DocumentCursor
  {
  private:
    Registry* registry_;
    kc::BasicDB::Cursor* cursor_;
    
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
  
  class Document
  {
  public:
    typedef std::vector<hash_t> hashes_vector;
    typedef std::bitset<(1<<24)> hashes_bloom;
    typedef std::map<std::string,std::string> content_map;
    
  private:
    uint32_t doctype_;
    uint32_t docid_;
    Registry* registry_;
    std::string* key_;
    std::string* content_;
    std::string* lower_case_;
    content_map* content_map_;
    hashes_vector* hashes_;
    hashes_vector* unique_sorted_hashes_;
    hashes_bloom* bloom_;
  
  public:
    Document(const uint32_t doctype,const uint32_t docid,const char* content,Registry* registry);
    Document(string& key,Registry* registry);
    
    ~Document();
    
    void clear();
    //Returns false if document already exists
    bool load();
    bool save();
    bool remove();
    
    hashes_vector& hashes();
    hashes_vector& unique_sorted_hashes();
    hashes_bloom& bloom();
    content_map& content();
    string& text();
    string& getLowerCase();
    string& getKey();
    string& title();
    uint64_t index_key();
    uint32_t windowsize();
    uint32_t doctype();
    uint32_t docid();
    
    void fill_document_dictionary(TemplateDictionary* dict);
    
    friend std::ostream& operator<< (std::ostream& stream, Document& document);
    
    friend bool operator< (Document& lhs,Document& rhs);
  };
  
}//namespace Superfastmatch

#endif                                   // duplication check