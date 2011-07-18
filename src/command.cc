#include "command.h"

namespace superfastmatch{
  const char* Command::key_format = "%d:%020d:%03d:%d:%010d:%010d"; 
  
  Command::Command(const Registry& registry,const string& key):
  registry_(registry),queue_id_(0),priority_(0),type_(Invalid),status_(Queued),doc_type_(0),doc_id_(0){
    if(sscanf(key.c_str(),key_format,&status_,&queue_id_,&priority_,&type_,&doc_type_,&doc_id_)!=6){
      throw "Bad parse of Command key!";
    }
  }
  
  Command::Command(const Registry& registry,const uint64_t queue_id, const uint32_t priority,const CommandType type,const CommandStatus status, const uint32_t doc_type,const uint32_t doc_id,const string& payload):
  registry_(registry),queue_id_(queue_id),priority_(priority),type_(type),status_(status),doc_type_(doc_type),doc_id_(doc_id){
    registry_.queueDB->set(command_key(),payload);
  }
  
  Command::~Command(){}
  
  string Command::command_key(){
    return kc::strprintf(key_format,status_,queue_id_,priority_,type_,doc_type_,doc_id_);
  }
  
  //Note that this method destroys the payload
  bool Command::update(){
    return registry_.queueDB->set(command_key(),"");
  }
  
  bool Command::remove(){
    return registry_.queueDB->remove(command_key());
  }
  
  bool Command::getPayload(string* payload){
    return registry_.queueDB->get(command_key(),payload);
  }
  
  CommandType Command::getType(){
    return type_;
  }
  
  CommandStatus Command::getStatus(){
    return status_;
  }
  
  uint32_t Command::getDocType(){
    return doc_type_;
  }
  
  uint32_t Command::getDocId(){
    return doc_id_;
  }
  
  uint64_t Command::getQueueId(){
    return queue_id_;
  }

  bool Command::setFinished(){
    if (not remove()){
      return false;
    }
    status_=Finished;
    return update();
  }
  
  bool Command::setFailed(){
    if (not remove()){
      return false;
    }
    status_=Failed;
    return update();
  }
  
  Document* Command::getDocument(){
    string payload;
    getPayload(&payload);
    Document* doc = new Document(doc_type_,doc_id_,payload.c_str(),registry_);
    // This may return false, ignore.
    // Important for drop document case
    doc->load();
    return doc;
  }
  
  void Command::fill_list_dictionary(TemplateDictionary* dict){
    switch (getStatus()){
      case Queued:
        dict->SetValue("STATUS","Queued");
        break;
      case Active:
        dict->SetValue("STATUS","Active");
        break;
      case Finished:
        dict->SetValue("STATUS","Finished");
        break;
      case Failed:
        dict->SetValue("STATUS","Failed");
        break;
    }
    switch (getType()){
      case Invalid:
        dict->SetValue("ACTION","Invalid");
        break;
      case AddDocument:
        dict->SetValue("ACTION","Add Document");
        break;
      case AddAssociation:
        dict->SetValue("ACTION","Add Association");
        break;
      case DropDocument:
        dict->SetValue("ACTION","Drop Document");
        break;
      case DropAssociation:
        dict->SetValue("ACTION","Drop Association");
        break;
    }
    dict->SetIntValue("ID",queue_id_);
    dict->SetIntValue("PRIORITY",priority_);
    dict->SetIntValue("DOC_TYPE",doc_type_);
    dict->SetIntValue("DOC_ID",doc_id_);
  }
  
  Command* CommandFactory::createCommand(const Registry& registry, const CommandType commandType,const uint64_t queue_id,const uint32_t doc_type, const uint32_t doc_id,const string& payload){
    switch (commandType){
      case AddDocument:
        return new Command(registry,queue_id,101,commandType,Queued,doc_type,doc_id,payload);
      case AddAssociation:
        return new Command(registry,queue_id,102,commandType,Queued,doc_type,doc_id,payload);
      case DropDocument:
        return new Command(registry,queue_id,2,commandType,Queued,doc_type,doc_id,payload);
      case DropAssociation:
        return new Command(registry,queue_id,1,commandType,Queued,doc_type,doc_id,payload);
      case Invalid:
        break;
    }
    throw "Invalid Command Type!";  
  }
  
  uint64_t CommandFactory::addDocument(const Registry& registry_,const uint32_t doc_type, const uint32_t doc_id,const string& content,const bool associate){
    uint64_t queue_id = registry_.miscDB->increment("QueueCounter",1);
    string empty;
    Command* add_doc = CommandFactory::createCommand(registry_,AddDocument,queue_id,doc_type,doc_id,content);
    delete add_doc;
    if (associate){
      Command* add_ass = CommandFactory::createCommand(registry_,AddAssociation,queue_id,doc_type,doc_id,empty);
      delete add_ass; 
    }
    return queue_id;
  }
    
  uint64_t CommandFactory::dropDocument(const Registry& registry_,const uint32_t doc_type, const uint32_t doc_id){
    uint64_t queue_id = registry_.miscDB->increment("QueueCounter",1);
    string empty;
    Command* drop_doc = CommandFactory::createCommand(registry_,DropDocument,queue_id,doc_type,doc_id,empty);
    Command* drop_ass = CommandFactory::createCommand(registry_,DropAssociation,queue_id,doc_type,doc_id,empty);
    delete drop_doc;
    delete drop_ass;
    return queue_id;
  }
  
  void CommandFactory::insertDropDocument(const Registry& registry_,Command* command){
    string empty;
    Command* drop_doc = CommandFactory::createCommand(registry_,DropDocument,command->getQueueId(),command->getDocType(),command->getDocId(),empty);
    Command* drop_ass = CommandFactory::createCommand(registry_,DropAssociation,command->getQueueId(),command->getDocType(),command->getDocId(),empty);
    delete drop_doc;
    delete drop_ass;
  }
  
  void CommandFactory::getAllCommands(const Registry& registry_,vector<Command*>& commands){
    kc::ForestDB::Cursor* cur = registry_.queueDB->cursor();
    cur->jump();
    string key;
    while (cur->get_key(&key,true)){
      Command* command = new Command(registry_,key);
      commands.push_back(command);
    }
    delete cur;
  }
  
  bool CommandFactory::getNextBatch(const Registry& registry_,deque<Command*>& batch,CommandType& batchType){
    string key;
    uint32_t batch_count=0;
    kc::ForestDB::Cursor* cur = registry_.queueDB->cursor();
    cur->jump();
    while (batch_count<registry_.max_batch_count && cur->get_key(&key,true)){
      batch_count++;
      Command* command = new Command(registry_,key);
      if (command->getStatus()!=Queued){
        delete command;
        command=0;
        break;
      }
      else if (batch.size()==0){
        batch.push_back(command);
        batchType=command->getType();
      }
      else if (command->getType()==batch.back()->getType()){
        batch.push_back(command); 
      }
      else{
        delete command;
        command=0;
        break;
      }
    }
    delete cur;
    return (batch.size()>0);
  } 
}