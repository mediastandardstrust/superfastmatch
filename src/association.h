#ifndef _SFMASSOCIATION_H                       // duplication check
#define _SFMASSOCIATION_H

#include "common.h"
#include "registry.h"
#include "templates.h"
#include "document.h"
#include "search.h"
#include "instrumentation.h"

namespace superfastmatch
{
  struct Match;
  struct Result;
  
  typedef unordered_set<uint32_t> hashes_set;
  typedef unordered_set<uint32_t> positions_set;
  typedef unordered_map<uint32_t,positions_set> matches_map;
  typedef pair<uint32_t,positions_set> match_pair;
  typedef deque<Match> matches_deque;
  typedef deque<Result> results_deque;
  
  struct Match{
     const uint32_t left;
     positions_set right;

     Match(uint32_t left):
     left(left)
     {}
  };
  
  struct Result{
    uint32_t left;
    uint32_t right;
    uint32_t length;
    uint32_t uc_left;
    uint32_t uc_right;
    uint32_t uc_length;
    uint32_t hash;
    
    Result(const uint32_t left,const uint32_t right,uint32_t length,const uint32_t uc_left,const uint32_t uc_right,uint32_t uc_length,const string& text):
    left(left),
    right(right),
    length(length),
    uc_left(uc_left),
    uc_right(uc_right),
    uc_length(uc_length),
    hash(hashmurmur(text.c_str(),text.length()))
    {}
  };

  class Association : public Instrumented<Association>
  {
  friend class AssociationManager;
  private:
    Registry* registry_;
    DocumentPtr from_document_;
    DocumentPtr to_document_;
    string* key_;
    string* reverse_key_;
    vector<Result>* results_;

  public:
    Association(Registry* registry,DocumentPtr from_document,DocumentPtr to_document);
    ~Association();

    void match();
    bool save();
    bool remove();
    string& getKey();
    string& getReverseKey();
    size_t getTotalLength();
    size_t getResultCount();
    const Result& getResult(size_t index);
    const string getFromResultText(size_t index);
    const string getToResultText(size_t index);
    void fillJSONDictionary(TemplateDictionary* dict,set<string>& metadata);
    
  private:
    bool load();
  };
  
  class AssociationManager
  {
  private:
    Registry* registry_;
    
  public:
    explicit AssociationManager(Registry* registry);
    
    vector<AssociationPtr> getAssociations(DocumentPtr doc,const int32_t state);
    bool removeAssociations(DocumentPtr doc);
  private:
    DISALLOW_COPY_AND_ASSIGN(AssociationManager);
  };
}//namespace Superfastmatch

#endif                                   // duplication check