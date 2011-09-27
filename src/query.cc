#include <query.h>

namespace superfastmatch
{
  DocTypeRange::DocTypeRange(const string& range){
    
  }
  
  DocumentQuery::DocumentQuery(Registry* registry, const string& command):
  registry_(registry),
  source_(command),
  target_(command)
  {
  }

  bool DocumentQuery::getOrderedDocPairs(vector<DocPair>& pairs){
    return false;
  }
  
  bool DocumentQuery::getUnorderedDocPairs(unordered_set<DocPair>& pairs){
    return false;
  }
}
