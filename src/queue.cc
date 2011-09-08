#include "queue.h"

namespace superfastmatch{
  QueueManager::QueueManager(Registry* registry):
  registry_(registry){}

  QueueManager::~QueueManager(){}

  CommandPtr QueueManager::createCommand(const CommandAction action,const uint32_t doc_type,const uint32_t doc_id,const string& payload){
    uint64_t queue_id=registry_->getMiscDB()->increment("QueueCounter",1);
    uint64_t payload_id=registry_->getMiscDB()->increment("PayloadCounter",1);
    return CommandPtr(new Command(registry_,action,queue_id,payload_id,doc_type,doc_id,payload));
  }

  CommandPtr QueueManager::insertCommand(const CommandAction action,const uint64_t queue_id,const uint32_t doc_type,const uint32_t doc_id,const string& payload){
    uint64_t payload_id=registry_->getMiscDB()->increment("PayloadCounter",1);
    return CommandPtr(new Command(registry_,action,queue_id,payload_id,doc_type,doc_id,payload));
  }

  CommandPtr QueueManager::getQueuedCommand(){
    CommandPtr command = CommandPtr();
    vector<string> keys;
    string value;
    if (registry_->getQueueDB()->match_prefix(toString(Queued),&keys,1)==1){
      registry_->getQueueDB()->get(keys[0],&value);
      command=getCommand(keys[0],value);
    }
    return command;
  }

  CommandPtr QueueManager::getCommand(const string& key,const string& value){
    return CommandPtr(new Command(registry_,key,value));
  }

  size_t QueueManager::processQueue(){
    size_t count=0;
    CommandAction previousAction=NullAction;
    CommandPtr command = getQueuedCommand();
    while (command){
      if (not command->execute()){
        assert(command->changeStatus(Queued));
      }else{
        count++;
        assert(command->changeStatus(Finished));
      }
      if((previousAction!=NullAction)&&(previousAction!=command->getAction())){
        registry_->getPostings()->wait(0);
      }
      // debug();
      previousAction=command->getAction();
      command = getQueuedCommand();
    }
    registry_->getPostings()->wait(0);
    return count;
  }

  void QueueManager::fillDictionary(TemplateDictionary* dict,const uint64_t cursor){
    TemplateDictionary* pager_dict=dict->AddIncludeDictionary("PAGING");
    pager_dict->SetFilename(PAGING);
    kc::PolyDB::Cursor* queue_cursor=registry_->getQueueDB()->cursor();
    size_t count;
    string start;
    vector<string> keys;
    string key;
    string value;
    CommandPtr command;
    if(queue_cursor->jump() && queue_cursor->get(&key,&value)){
      command = getCommand(key,value);
      pager_dict->SetValueAndShowSection("PAGE",toString(command->getQueueId()),"FIRST");
    }
    if(queue_cursor->jump_back()){
      count=registry_->getPageSize()-1;
      while(count--){ 
        queue_cursor->step_back();
      }
      if (queue_cursor->get(&key,&value)){
        command = getCommand(key,value);
        pager_dict->SetValueAndShowSection("PAGE",toString(command->getQueueId()),"LAST");
        start=key; 
      }
    }
    if (cursor && (registry_->getQueueDB()->match_prefix(kc::strprintf("%u:%020lu",Queued,cursor),&keys,1)==1 ||\
                   registry_->getQueueDB()->match_prefix(kc::strprintf("%u:%020lu",Active,cursor),&keys,1)==1 ||\
                   registry_->getQueueDB()->match_prefix(kc::strprintf("%u:%020lu",Failed,cursor),&keys,1)==1 ||\
                   registry_->getQueueDB()->match_prefix(kc::strprintf("%u:%020lu",Finished,cursor),&keys,1)==1 ||\
                   registry_->getQueueDB()->match_prefix(kc::strprintf("%u",Active),&keys,1)==1 ||\
                   registry_->getQueueDB()->match_prefix(kc::strprintf("%u",Queued),&keys,1)==1))
    {   
      start=keys[0];
    }
    queue_cursor->jump(start);
    count=registry_->getPageSize();
    while(count--){
      queue_cursor->step_back();
    }
    if(queue_cursor->get(&key,&value)){
      command = getCommand(key,value);
      pager_dict->SetValueAndShowSection("PAGE",toString(command->getQueueId()),"PREVIOUS");
    }
    queue_cursor->jump(start);
    count=registry_->getPageSize();
    while(count>0 && queue_cursor->get(&key,&value,true)){
      command = getCommand(key,value);
      command->fillDictionary(dict);
      count--;
    }
    if(queue_cursor->get(&key,&value)){
      command = getCommand(key,value);
      pager_dict->SetValueAndShowSection("PAGE",toString(command->getQueueId()),"NEXT");
    }
    delete queue_cursor;
  }

  void QueueManager::debug(){
    string key;
    string value;
    kc::PolyDB::Cursor* cursor=registry_->getQueueDB()->cursor();
    cursor->jump();
    while(cursor->get(&key,&value,true)){
      cout << getCommand(key,value)->getKey() << endl;
    }
    cout << "--------------------------------------------------" <<endl;
    delete cursor;
  }
}