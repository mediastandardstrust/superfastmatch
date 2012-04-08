#include "queue.h"

namespace superfastmatch{
	RegisterTemplateFilename(QUEUE_JSON, "JSON/queue.tpl");

  QueueManager::QueueManager(Registry* registry):
  registry_(registry){}

  QueueManager::~QueueManager(){}

  CommandPtr QueueManager::createCommand(const CommandAction action,const uint32_t doc_type,const uint32_t doc_id,const string& source, const string& target, const string& payload){
    uint64_t queue_id=registry_->getMiscDB()->increment("QueueCounter",1);
    uint64_t payload_id=registry_->getMiscDB()->increment("PayloadCounter",1);
    return CommandPtr(new Command(registry_,action,queue_id,payload_id,doc_type,doc_id,source,target,payload));
  }

  CommandPtr QueueManager::insertCommand(const CommandAction action,const uint64_t queue_id,const uint32_t doc_type,const uint32_t doc_id, const string& source, const string& target,const string& payload){
    uint64_t payload_id=registry_->getMiscDB()->increment("PayloadCounter",1);
    return CommandPtr(new Command(registry_,action,queue_id,payload_id,doc_type,doc_id,source,target,payload));
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
    bool active=false;
    CommandAction previousAction=NullAction;
    CommandPtr command = getQueuedCommand();
    while (command && !registry_->isClosing()){
      active=true;
      if((previousAction!=NullAction)&&(previousAction!=command->getAction())){
        registry_->getPostings()->finishTasks();
      }
      if (not command->execute()){
        assert(command->changeStatus(Queued));
      }else{
        count++;
        assert(command->changeStatus(Finished));
      }
      // debug();
      previousAction=command->getAction();
      command = getQueuedCommand();
    }
    if (active){
      registry_->getPostings()->finishTasks(); 
    }
    return count;
  }
  
  void QueueManager::fillDictionary(TemplateDictionary* dict,const uint64_t start,const uint64_t limit){
    TemplateDictionary* queueDict=dict->AddIncludeDictionary("DATA");
    queueDict->SetFilename(QUEUE_JSON);
    queueDict->SetIntValue("TOTAL",registry_->getQueueDB()->count());
    kc::PolyDB::Cursor* queue_cursor=registry_->getQueueDB()->cursor();
    size_t count;
    string cursor;
    vector<string> keys;
    string key;
    string value;
    CommandPtr command;
    if(queue_cursor->jump() && queue_cursor->get(&key,&value)){
      command = getCommand(key,value);
      queueDict->SetIntValue("FIRST",command->getQueueId());
    }
    if(queue_cursor->jump_back()){
      count=limit-1;
      while(count--){ 
        queue_cursor->step_back();
      }
      if (queue_cursor->get(&key,&value)){
        command = getCommand(key,value);
        queueDict->SetIntValue("LAST",command->getQueueId());
        cursor=key; 
      }
    }
    if (start && (registry_->getQueueDB()->match_prefix(kc::strprintf("%u|%020lu",Queued,start),&keys,1)==1 ||\
                  registry_->getQueueDB()->match_prefix(kc::strprintf("%u|%020lu",Active,start),&keys,1)==1 ||\
                  registry_->getQueueDB()->match_prefix(kc::strprintf("%u|%020lu",Failed,start),&keys,1)==1 ||\
                  registry_->getQueueDB()->match_prefix(kc::strprintf("%u|%020lu",Finished,start),&keys,1)==1 ||\
                  registry_->getQueueDB()->match_prefix(kc::strprintf("%u",Active),&keys,1)==1 ||\
                  registry_->getQueueDB()->match_prefix(kc::strprintf("%u",Queued),&keys,1)==1))
    {   
      cursor=keys[0];
    }
    queue_cursor->jump(cursor);
    count=limit;
    while(count--){
      queue_cursor->step_back();
    }
    if(queue_cursor->get(&key,&value)){
      command = getCommand(key,value);
      queueDict->SetIntValue("PREVIOUS",command->getQueueId());
    }
    queue_cursor->jump(cursor);
    count=limit;
    while(count>0 && queue_cursor->get(&key,&value,true)){
      command = getCommand(key,value);
      command->fillDictionary(queueDict);
      count--;
    }
    if(queue_cursor->get(&key,&value)){
      command = getCommand(key,value);
      queueDict->SetIntValue("NEXT",command->getQueueId());
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