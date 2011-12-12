#include <search.h>
#include "document.h"
#include "association.h"

namespace superfastmatch{
  RegisterTemplateFilename(ASSOCIATION, "association.tpl");
  RegisterTemplateFilename(SEARCH_JSON, "JSON/search.tpl");

  Search::Search(Registry* registry,DocumentPtr doc,const string& name):
  registry_(registry),
  doc(doc),
  performance(new InstrumentGroup(name,registry->getNumResults(),registry->getNumResults()))
  {
    performance->add(doc->getInstrument());
  }

  SearchPtr Search::createTemporarySearch(Registry* registry,const string& text){
    DocumentPtr doc=registry->getDocumentManager()->createTemporaryDocument(text);
    SearchPtr search=SearchPtr(new Search(registry,doc,"Create Temporary Search"));
    search->execute();
    return search;
  }

  SearchPtr Search::createAnonymousSearch(Registry* registry,const string& text){
    DocumentPtr doc=registry->getDocumentManager()->createTemporaryDocument(text);
    SearchPtr search=SearchPtr(new Search(registry,doc,"Create Anonymous Search"));
    search->execute();
    return search;
  }
  
  SearchPtr Search::createPermanentSearch(Registry* registry,const uint32_t doctype,const uint32_t docid){
    DocumentPtr doc=registry->getDocumentManager()->getDocument(doctype,docid);
    SearchPtr search=SearchPtr(new Search(registry,doc,"Create Permanent Search"));
    search->execute();
    return search;
  }

  SearchPtr Search::getPermanentSearch(Registry* registry,const uint32_t doctype,const uint32_t docid){
    DocumentPtr doc=registry->getDocumentManager()->getDocument(doctype,docid,DocumentManager::TEXT|DocumentManager::META);
    SearchPtr search=SearchPtr(new Search(registry,doc,"Get Permanent Search"));
    search->associations=registry->getAssociationManager()->getAssociations(doc,DocumentManager::META);
    return search;
  }
  
  void Search::execute(){
    size_t num_results=registry_->getNumResults();
    size_t count=0;
    registry_->getPostings()->searchIndex(*this);
    for(inverted_search_t::iterator it2=pruned_results.begin(),ite2=pruned_results.end();(it2!=ite2) && (count<num_results);++it2){
      DocumentPtr other=registry_->getDocumentManager()->getDocument(it2->second.doc_type,it2->second.doc_id,DocumentManager::META);
      AssociationPtr association(new Association(registry_,doc,other));
      performance->add(association->getInstrument());
      associations.push_back(association);
      if (doc->isPermanent()){
        association->save();
      }
      count++;
    }
  }
  
  void Search::fillDictionary(TemplateDictionary* dict){
    TemplateDictionary* association_dict=dict->AddIncludeDictionary("ASSOCIATION");
    association_dict->SetFilename(ASSOCIATION);
    for (vector<AssociationPtr>::iterator it=associations.begin(),ite=associations.end();it!=ite;++it){
      (*it)->fillItemDictionary(association_dict);
    }
  }

  void Search::fillJSONDictionary(TemplateDictionary* dict,const bool includeDoc){
    TemplateDictionary* searchDict=dict->AddIncludeDictionary("DATA");
    searchDict->SetFilename(SEARCH_JSON);
    for (vector<AssociationPtr>::iterator it=associations.begin(),ite=associations.end();it!=ite;++it){
      (*it)->fillJSONDictionary(searchDict);
    }
    if(includeDoc){
      TemplateDictionary* sourceDict=searchDict->AddSectionDictionary("SOURCE");
      sourceDict->SetValue("TEXT",doc->getText());
    }
  }
}