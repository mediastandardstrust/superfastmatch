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

  class DocumentQuery
  {
  private:
    typedef unordered_set<uint32_t>* set_t;
    Registry* registry_;
    set_t left_doc_types_;
    set_t left_doc_ids_;
    set_t right_doc_types_;
    set_t right_doc_ids_;
    string cursor_;
    string order_by_;
    size_t offset;
    size_t limit;
    // const Document::DocumentOrder order_;
  
    DocumentQuery(Registry& registry, const string& command="",const size_t limit=0);
    ~DocumentQuery();
  
    bool getOrderedDocPairs(vector<DocPair>& pairs);
    bool getUnorderedDocPairs(unordered_set<DocPair>& pairs);
  };
}
#endif