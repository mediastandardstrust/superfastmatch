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
    
    DocPair():
    doc_type(0),
    doc_id(0)
    {}
    
    DocPair(uint32_t doc_type,uint32_t doc_id):
    doc_type(doc_type),
    doc_id(doc_id)
    {}
    
    DocPair(const string& key){
      assert(key.size()==8);
      memcpy(&doc_type,key.data(),4);
      memcpy(&doc_id,key.data()+4,4);
      doc_type=kc::ntoh32(doc_type);
      doc_id=kc::ntoh32(doc_id);
    }
  };
  
  typedef struct{
    inline size_t operator() (const DocPair& k) const {
      return (static_cast<uint64_t>(k.doc_type)<<32)|k.doc_id;
    }
  } DocPairHash;

  typedef struct
  {
    inline bool operator() (const DocPair& x, const DocPair &y) const { 
      return (x.doc_id==y.doc_id)&(x.doc_type==y.doc_type);
      // return (x.doc_id==y.doc_id) && (x.doc_type==y.doc_type);
    }
  } DocPairEq;

  class DocTypeRange{
    set<uint32_t> doctypes_;

    public:
      explicit DocTypeRange();
      bool parse(const string& range);
      bool isInRange(uint32_t doctype) const;
      set<uint32_t> getDocTypes();
  };

  class DocumentQuery
  {
  private:
    Registry* registry_;
    DocTypeRange source_;
    DocTypeRange target_;
    vector<DocPair> source_pairs_;
    vector<DocPair> target_pairs_;
    string path_;
    string cursor_;
    string order_by_;
    string next_;
    string previous_;
    string first_;
    string last_;
    uint64_t limit_;
    bool desc_;
    bool valid_;

  public:
    explicit DocumentQuery(Registry* registry, const string& command="");
  
    bool isValid();
    const vector<DocPair>& getSourceDocPairs(bool unlimited=false);
    const vector<DocPair>& getTargetDocPairs(bool unlimited=false);
    const string& getCursor() const;
    const string& getOrder() const;
    const string& getFirst();
    const string& getLast();
    const string& getPrevious();
    const string& getNext();
    bool isDescending();
    uint64_t getLimit();
    void fillListDictionary(TemplateDictionary* dict);

  private:
    const string getCursor(const DocPair& pair)const;
    const string parseCursor(const string& cursor)const;
    const string getCommand() const;
    const string getCommand(const DocPair& pair) const;
    vector<DocPair> getDocPairs(const DocTypeRange& range, const string& order_by,const string& cursor, const uint64_t limit, const bool desc) const;
    DISALLOW_COPY_AND_ASSIGN(DocumentQuery);
  };
}
#endif
