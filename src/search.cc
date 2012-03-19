#include <search.h>
#include "document.h"
#include "association.h"

namespace superfastmatch{
  RegisterTemplateFilename(SEARCH_JSON, "JSON/search.tpl");
  RegisterTemplateFilename(DOCUMENTS_JSON, "JSON/documents.tpl");

  Search::Search(Registry* registry,DocumentPtr doc,DocumentQueryPtr target,const string& name):
  registry_(registry),
  doc(doc),
  target(target),
  performance(new InstrumentGroup(name,registry->getNumResults(),registry->getNumResults()))
  {
    performance->add(doc->getInstrument());
  }

  SearchPtr Search::createTemporarySearch(Registry* registry,const string& text,DocumentQueryPtr target){
    DocumentPtr doc=registry->getDocumentManager()->createTemporaryDocument(text);
    SearchPtr search=SearchPtr(new Search(registry,doc,target,"Create Temporary Search"));
    search->execute();
    return search;
  }

  SearchPtr Search::createAnonymousSearch(Registry* registry,const string& text,DocumentQueryPtr target){
    DocumentPtr doc=registry->getDocumentManager()->createTemporaryDocument(text);
    SearchPtr search=SearchPtr(new Search(registry,doc,target,"Create Anonymous Search"));
    search->execute();
    return search;
  }
  
  SearchPtr Search::createPermanentSearch(Registry* registry,const uint32_t doctype,const uint32_t docid,DocumentQueryPtr target){
    DocumentPtr doc=registry->getDocumentManager()->getDocument(doctype,docid);
    SearchPtr search=SearchPtr(new Search(registry,doc,target,"Create Permanent Search"));
    search->execute();
    return search;
  }

  SearchPtr Search::getPermanentSearch(Registry* registry,const uint32_t doctype,const uint32_t docid){
    DocumentPtr doc=registry->getDocumentManager()->getDocument(doctype,docid,DocumentManager::TEXT|DocumentManager::META);
    SearchPtr search;
    if (doc){
      DocumentQueryPtr target(new DocumentQuery(registry,"",""));
      search=SearchPtr(new Search(registry,doc,target,"Get Permanent Search"));
      search->associations=registry->getAssociationManager()->getAssociations(doc,DocumentManager::META);      
    }
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
      if (association->getResultCount()>0){
        associations.push_back(association);
        if (doc->isPermanent()){
          association->save();
        }
        count++;  
      }
    }
  }
  
  void Search::fillJSONDictionary(TemplateDictionary* dict,const bool includeDoc){
    TemplateDictionary* searchDict=dict->AddIncludeDictionary("DATA");
    searchDict->SetFilename(SEARCH_JSON);
    TemplateDictionary* documentsDict=searchDict->AddIncludeDictionary("DOCUMENTS");
    documentsDict->SetFilename(DOCUMENTS_JSON);
    set<string> metadata;
    for (vector<AssociationPtr>::iterator it=associations.begin(),ite=associations.end();it!=ite;++it){
      (*it)->fillJSONDictionary(documentsDict,metadata);
    }
    for (set<string>::const_iterator it=metadata.begin(),ite=metadata.end();it!=ite;++it){
      TemplateDictionary* fields_dict=documentsDict->AddSectionDictionary("FIELDS");
      fields_dict->SetValue("FIELD",*it);
    }
    if(includeDoc){
      TemplateDictionary* sourceDict=searchDict->AddSectionDictionary("SOURCE");
      sourceDict->SetValue("TEXT",doc->getText());
    }
  }
}