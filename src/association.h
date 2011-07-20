#ifndef _SFMASSOCIATION_H                       // duplication check
#define _SFMASSOCIATION_H

#include "common.h"
#include "registry.h"
#include "document.h"
#include "templates.h"

namespace superfastmatch
{
  struct Match;
  struct Result;
  
  typedef std::tr1::unordered_set<hash_t> hashes_set;
  typedef std::tr1::unordered_set<uint32_t> positions_set;
  typedef std::tr1::unordered_map<hash_t,positions_set> matches_map;
  typedef std::pair<uint32_t,positions_set> match_pair;
  typedef std::deque<Match> matches_deque;
  typedef std::deque<Result> results_deque;
  
  struct Match{
     uint32_t left;
     positions_set right;
     Match(uint32_t left,positions_set right) :left(left), right(right){}
  };
  
  struct Result{
     uint32_t left;
     uint32_t right;
     uint32_t length;
     Result(uint32_t left,uint32_t right,uint32_t length) : left(left), right(right), length(length){}
  };

  class Association
  {
  private:
    const Registry& registry_;
    vector<Result> results_;
    Document* from_document_;
    Document* to_document_;
    
  public:
    Association(const Registry& registry,Document* from_document,Document* to_document);
    
    bool load();
    void fill_item_dictionary(TemplateDictionary* dict);
    void fill_list_dictionary(TemplateDictionary* dict);
  private:
    bool save();
    void match();
  };
}//namespace Superfastmatch

#endif                                   // duplication check