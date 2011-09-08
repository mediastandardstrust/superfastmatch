#ifndef _SFMQUEUE_H                       // duplication check
#define _SFMQUEUE_H

#include <common.h>
#include <registry.h>
#include <document.h>
#include <command.h>
#include <posting.h>

namespace superfastmatch
{   
  class QueueManager{
  private:
    Registry* registry_;

  public:
    QueueManager(Registry* registry);
    ~QueueManager();
    CommandPtr insertCommand(const CommandAction action,const uint64_t queue_id,const uint32_t doc_type,const uint32_t doc_id,const string& payload);
    CommandPtr createCommand(const CommandAction action,const uint32_t doc_type,const uint32_t doc_id,const string& payload);
    CommandPtr getQueuedCommand();
    size_t processQueue();
    void fillDictionary(TemplateDictionary* dict,const uint64_t cursor=0);

  private:
    CommandPtr getCommand(const string& key,const string& value);
    void debug();
    DISALLOW_COPY_AND_ASSIGN(QueueManager);
  };
}

#endif