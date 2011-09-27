#ifndef _SFMQUERY_H                       // duplication check
#define _SFMQUERY_H

#include <common.h>
#include <registry.h>

using namespace std;
using namespace std::tr1;

namespace superfastmatch
{
  struct DocPair{
    uint32_t doc_type;
    uint32_t doc_id;
    DocPair(uint32_t doc_type,uint32_t doc_id):
    doc_type(doc_type),
    doc_id(doc_id)
    {}
  };
  
  typedef struct{
    size_t operator() (const DocPair& k) const { 
      return (static_cast<uint64_t>(k.doc_type)<<32)|k.doc_id;
    }
  } DocPairHash;

  typedef struct
  {
    bool operator() (const DocPair& x, const DocPair &y) const { return (x.doc_type==y.doc_type) && (x.doc_id==y.doc_id); }
  } DocPairEq;

  class DocTypeRange{
    set<uint32_t> doctypes_;
    
    public:
      explicit DocTypeRange(const string& range);
      bool isInRange(uint32_t doctype);
      void fillItemDictionary(TemplateDictionary* dict);
  };

  class DocumentQuery
  {
  private:
    Registry* registry_;
    DocTypeRange source_;
    DocTypeRange target_;
    string cursor_;
    string order_by_;
    size_t offset_;
    size_t limit_;
    // const Document::DocumentOrder order_;

  public:
    explicit DocumentQuery(Registry* registry, const string& command="");
  
    bool getOrderedDocPairs(vector<DocPair>& pairs);
    bool getUnorderedDocPairs(unordered_set<DocPair>& pairs);
  
  private:
    DISALLOW_COPY_AND_ASSIGN(DocumentQuery);
  };
}
#endif