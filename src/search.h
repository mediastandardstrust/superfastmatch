#ifndef _SFMSEARCH_H                       // duplication check
#define _SFMSEARCH_H

#include "common.h"
#include "instrumentation.h"
#include "query.h"

namespace superfastmatch{
  // Forward Declarations
  class Search;
  typedef shared_ptr<Search> SearchPtr;
  
  struct DocTally{
    uint64_t count;
    uint64_t previous;
    DocTally():
    count(0),
    previous(0)
    {}
  };

  typedef struct
  {
    bool operator()(const DocTally &lhs, const DocTally &rhs) const { 
      return lhs.count>rhs.count;
    }
  } DocTallyEq;

  typedef unordered_map<DocPair,DocTally,DocPairHash,DocPairEq> search_t;
  typedef multimap<DocTally,DocPair,DocTallyEq> inverted_search_t;

  class Search{
  private:
    Registry* registry_;
    Search(Registry* registry_,DocumentPtr doc,const string& name);
    void execute();
    DISALLOW_COPY_AND_ASSIGN(Search);
  public:
    DocumentPtr doc;
    search_t results;
    inverted_search_t pruned_results;
    vector<AssociationPtr> associations;
    InstrumentGroupPtr performance;
    
    void fillDictionary(TemplateDictionary* dict);
    
    // Factory methods
    static SearchPtr createTemporarySearch(Registry* registry,const string& text);
    static SearchPtr createAnonymousSearch(Registry* registry,const string& text);
    static SearchPtr createPermanentSearch(Registry* registry,const uint32_t doctype,const uint32_t docid);
    static SearchPtr getPermanentSearch(Registry* registry,const uint32_t doctype,const uint32_t docid);
  };  
}
#endif