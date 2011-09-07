#include "command.h"
#include "queue.h"

namespace superfastmatch{
  const Command::action_map Command::actions=create_map<CommandAction,ActionDetail>(DropDocument,Command::ActionDetail(1,&Command::dropDocument,"Drop Document"))\
                                                                                   (AddDocument,Command::ActionDetail(2,&Command::addDocument,"Add Document"))\
                                                                                   (AddAssociations,Command::ActionDetail(3,&Command::addAssociations,"Add Associations"))\
                                                                                   (AddAssociation,Command::ActionDetail(4,&Command::addAssociation,"Add Association"));

  const Command::status_map Command::statuses=create_map<CommandStatus,string>(Queued,"Queued")(Active,"Active")(Finished,"Finished")(Failed,"Failed");

  const char* Command::key_format    = "%u:%020lu:%03u:%u:%010u:%010u";
  const char* Command::cursor_format = "%u:%020lu";
  
  Command::Command(Registry* registry,const CommandAction action,const uint64_t queue_id,const uint64_t payload_id,const uint32_t doc_type,const uint32_t doc_id,const string& payload):
  registry_(registry),
  queue_id_(queue_id),
  payload_id_(payload_id),
  status_(Queued),
  action_(action),
  doc_type_(doc_type),
  doc_id_(doc_id),
  payload_(new string(payload))
  {
    action_map::const_iterator action_it=actions.find(action);
    if (action_it!=actions.end()){
      priority_=action_it->second.priority;
    }else{
      throw("Bad action");
    }
    assert(save());
  }
  
  Command::Command(Registry* registry,const string& key,const string& value):
  registry_(registry),
  payload_(0)
  {
    if(sscanf(key.c_str(),key_format,&status_,&queue_id_,&priority_,&action_,&doc_type_,&doc_id_)!=6){
      throw "Bad parse of Command key!";
    }
    payload_id_=kc::atoi(value.c_str());
  }
  
  Command::~Command(){
    delete payload_;
  }
  
  string Command::getKey(){
    return kc::strprintf(key_format,status_,queue_id_,priority_,action_,doc_type_,doc_id_);
  }
  
  uint64_t Command::getQueueId(){
    return queue_id_;
  }
  
  uint64_t Command::getPayloadId(){
    return payload_id_;
  }
  
  bool Command::save(){
    return registry_->getQueueDB()->set(getKey(),toString(payload_id_)) &&\
           registry_->getPayloadDB()->set(toString(payload_id_),*payload_);
  }
  
  bool Command::changeStatus(CommandStatus status){
    if (not registry_->getQueueDB()->remove(getKey())){
      return false;
    }
    status_=status;
    return registry_->getQueueDB()->set(getKey(),toString(payload_id_));
  }
  
  bool Command::execute(){
    assert(changeStatus(Active));
    action_map::const_iterator action=actions.find(action_);
    if (action!=actions.end()){
      return (this->*action->second.method)();
    }
    return false;
  }
  
  bool Command::addDocument(){
    DocumentPtr doc = registry_->getDocumentManager()->createPermanentDocument(getDocType(),getDocId(),getPayload());
    if (doc){
      registry_->getPostings()->addDocument(doc);
      return true;
    }
    registry_->getQueueManager()->insertCommand(DropDocument,shared_from_this());
    return false;
  }
  
  bool Command::addAssociation(){
    DocumentPtr doc=registry_->getDocumentManager()->getDocument(getDocType(),getDocId());
    registry_->getDocumentManager()->associateDocument(doc);
    return true;
  }

  bool Command::addAssociations(){
    // vector<DocumentPtr> documents=registry_->getDocumentManager()->getDocuments(getDocType(),getDocId());
    // for(vector<DocumentPtr>::iterator it=documents.begin(),ite=documents.end();it!=ite;++it){
    //   registry_->getQueueManager()->insertCommand(AssociateDocument,(*it)->getQueueId(),(*it)->getDocType(),(*it)->getDocid());
    // }
    return true;
  }
  
  bool Command::dropDocument(){
    DocumentPtr doc=registry_->getDocumentManager()->getDocument(getDocType(),getDocId());
    assert(registry_->getAssociationManager()->removeAssociations(doc));
    assert(registry_->getPostings()->deleteDocument(doc));
    return registry_->getDocumentManager()->removePermanentDocument(doc);
  }
  
  uint32_t Command::getDocType(){
    return doc_type_;
  }
  
  uint32_t Command::getDocId(){
    return doc_id_;
  }
  
  string& Command::getPayload(){
    if (payload_==0){
      payload_=new string();
      assert(registry_->getPayloadDB()->get(toString(payload_id_),payload_));
    }
    return *payload_;
  }
  
  CommandAction Command::getAction(){
    return action_;
  }
  
  CommandStatus Command::getStatus(){
    return status_;
  }
  
  void Command::fillDictionary(TemplateDictionary* dict){
    TemplateDictionary* command_dict=dict->AddSectionDictionary("COMMAND");
    command_dict->SetValue("STATUS",statuses.find(status_)->second);
    command_dict->SetValue("ACTION",actions.find(action_)->second.name);
    command_dict->SetIntValue("ID",queue_id_);
    command_dict->SetIntValue("PRIORITY",priority_);
    command_dict->SetIntValue("DOC_TYPE",doc_type_);
    command_dict->SetIntValue("DOC_ID",doc_id_);
  }
}