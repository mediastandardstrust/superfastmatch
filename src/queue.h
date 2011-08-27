#ifndef _SFMQUEUE_H                       // duplication check
#define _SFMQUEUE_H

#include <map>
#include <iomanip>
#include <sstream>
#include <deque>
#include <registry.h>
#include <document.h>
#include <command.h>
#include <posting.h>

namespace superfastmatch
{   
  class Queue{
  private:
    Registry* registry_;
  public:
    Queue(Registry* registry);
  
    uint64_t add_document(const uint32_t doc_type,const uint32_t doc_id,const string& content,bool associate);
    uint64_t delete_document(const uint32_t& doc_type,const uint32_t& doc_id);
    uint64_t addAssociations(const uint32_t& doc_type=0);
    
    bool process();
    bool purge();
    
    void fill_list_dictionary(TemplateDictionary* dict);
    void fill_item_dictionary(TemplateDictionary* dict,const uint64_t queue_id);
  };

}

#endif