#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <document.h>
#include <posting.h>
#include <kcprotodb.h>
#include <mock_registry.h>
#include <templates.h>

using namespace testing;
using namespace superfastmatch;
using namespace kyotocabinet;

class CommandTest :public ::testing::Test{
protected:
  PolyDB* associationDB_;
  PolyDB* documentDB_;
  PolyDB* metaDB_;
  PolyDB* queueDB_;
  PolyDB* payloadDB_;
  PolyDB* miscDB_;
  DocumentManager* documentManager_;
  AssociationManager* associationManager_;
  Posting* postings_;
  MockRegistry registry_;
  Logger* logger_;
  
  virtual void SetUp(){
    associationDB_ = new PolyDB();
    documentDB_ = new PolyDB();
    metaDB_ = new PolyDB();
    queueDB_ = new PolyDB();
    payloadDB_ = new PolyDB();
    miscDB_ = new PolyDB();
    associationDB_->open("%");
    documentDB_->open("%");
    metaDB_->open("%");
    queueDB_->open("%");
    payloadDB_->open("%");
    miscDB_->open("%");
    EXPECT_CALL(registry_,getWindowSize())
      .WillRepeatedly(Return(4));
    EXPECT_CALL(registry_,getSlotCount())
      .WillRepeatedly(Return(4));
    EXPECT_CALL(registry_,getWhiteSpaceThreshold())
      .WillRepeatedly(Return(4));
    EXPECT_CALL(registry_,getWhiteSpaceHash(false))
      .WillRepeatedly(Return(0));
    EXPECT_CALL(registry_,getWhiteSpaceHash(true))
      .WillRepeatedly(Return(0));
    EXPECT_CALL(registry_,getMaxPostingThreshold())
      .WillRepeatedly(Return(200));
    EXPECT_CALL(registry_,getMaxDistance())
      .WillRepeatedly(Return(5));
    EXPECT_CALL(registry_,getSlotCount())
      .WillRepeatedly(Return(4));
    EXPECT_CALL(registry_,getHashWidth())
      .WillRepeatedly(Return(24));
    EXPECT_CALL(registry_,getHashMask())
      .WillRepeatedly(Return((1L<<24)-1));
    EXPECT_CALL(registry_,getMaxHashCount())
      .WillRepeatedly(Return(1<<24));
    EXPECT_CALL(registry_,getMaxLineLength())
      .WillRepeatedly(Return(1024));
    EXPECT_CALL(registry_,getNumResults())
      .WillRepeatedly(Return(20));
    EXPECT_CALL(registry_,getPageSize())
      .WillRepeatedly(Return(100));  
      
    EXPECT_CALL(registry_,getMetaDB())
      .WillRepeatedly(Return(metaDB_));
    EXPECT_CALL(registry_,getMiscDB())
      .WillRepeatedly(Return(miscDB_));
    EXPECT_CALL(registry_,getDocumentDB())
      .WillRepeatedly(Return(documentDB_));
    EXPECT_CALL(registry_,getAssociationDB())
      .WillRepeatedly(Return(associationDB_));
    EXPECT_CALL(registry_,getQueueDB())
      .WillRepeatedly(Return(queueDB_));
    EXPECT_CALL(registry_,getPayloadDB())
      .WillRepeatedly(Return(payloadDB_));

    documentManager_ = new DocumentManager(&registry_);
    associationManager_ = new AssociationManager(&registry_);
    postings_ = new Posting(&registry_);
    logger_ = new Logger();
    EXPECT_CALL(registry_,getDocumentManager())
      .WillRepeatedly(Return(documentManager_));
    EXPECT_CALL(registry_,getAssociationManager())
      .WillRepeatedly(Return(associationManager_));
    EXPECT_CALL(registry_,getPostings())
      .WillRepeatedly(Return(postings_));
    EXPECT_CALL(registry_,getLogger())
      .WillRepeatedly(Return(logger_));
  }
  
  virtual void TearDown(){
    associationDB_->close();
    metaDB_->close();
    documentDB_->close();
    queueDB_->close();
    payloadDB_->close();
    miscDB_->close();
    delete associationDB_;
    delete metaDB_;
    delete documentDB_;
    delete queueDB_;
    delete payloadDB_;
    delete miscDB_;
    delete documentManager_;
    delete associationManager_;
    delete postings_;
    delete logger_;
  }
};

// Testing!
namespace superfastmatch{
  // Stick in common.h
  class Command;
  typedef std::tr1::shared_ptr<Command> CommandPtr;

  // Stick in command.h
  enum CommandAction{
    AddDocument=1,
    AddAssociation,
    AddAssociations,
    DropDocument,
    DropAssociation,
    NullAction
  };

  enum CommandStatus{
    Finished=1,
    Failed,
    Active,
    Queued
  };

  class Command : public std::tr1::enable_shared_from_this<Command>
  {
  friend class QueueManager;
  public:
    typedef bool(Command::*Method)();
    struct ActionDetail{
      uint32_t priority;
      Method method;
      string name;
      ActionDetail(){};
      ActionDetail(const uint32_t priority, const Method method,const string name):
      priority(priority),
      method(method),
      name(name){};
    };
    typedef map<CommandAction,Command::ActionDetail> action_map;
    typedef map<CommandStatus,string> status_map;
  private:
    Registry* registry_;
    uint64_t queue_id_;
    uint64_t payload_id_;
    uint32_t priority_;
    CommandStatus status_;
    CommandAction action_;
    uint32_t doc_type_;
    uint32_t doc_id_;
    string* payload_;

    static const char* key_format;
    static const char* cursor_format;
    static const action_map actions;
    static const status_map statuses;

    // Command methods
    bool addDocument();
    bool addAssociation();
    bool addAssociations();
    bool dropDocument();
    
    bool save();
    bool changeStatus(CommandStatus status);

  public:
    ~Command();
    bool execute();
    string getKey();
    string getCursorKey();
    uint32_t getDocType();
    uint32_t getDocId();
    uint64_t getQueueId();
    uint64_t getPayloadId();
    string& getPayload();
    CommandAction getAction();
    CommandStatus getStatus();
    void fillDictionary(TemplateDictionary* dict);

  private:
    Command(Registry* registry,const CommandAction action,const uint64_t queue_id,const uint64_t payload_id,const uint32_t doc_type,const uint32_t doc_id,const string& payload);
    Command(Registry* registry,const string& key,const string& value);
    DISALLOW_COPY_AND_ASSIGN(Command);
  };

  const Command::action_map Command::actions=create_map<CommandAction,ActionDetail>(DropDocument,Command::ActionDetail(1,&Command::dropDocument,"Drop Document"))\
                                                                                   (AddDocument,Command::ActionDetail(2,&Command::addDocument,"Add Document"))\
                                                                                   (AddAssociations,Command::ActionDetail(3,&Command::addAssociations,"Add Associations"))\
                                                                                   (AddAssociation,Command::ActionDetail(4,&Command::addAssociation,"Add Association"));

  const Command::status_map Command::statuses=create_map<CommandStatus,string>(Queued,"Queued")(Active,"Active")(Finished,"Finished")(Failed,"Failed");

  const char* Command::key_format    = "%d:%020d:%03d:%d:%010d:%010d";
  const char* Command::cursor_format = "%d:%020d";
  
  Command::Command(Registry* registry,const CommandAction action,const uint64_t queue_id,const uint64_t payload_id,const uint32_t doc_type,const uint32_t doc_id,const string& payload):
  registry_(registry),
  status_(Queued),
  action_(action),
  doc_type_(doc_type),
  doc_id_(doc_id),
  queue_id_(queue_id),
  payload_id_(payload_id),
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
  
  string Command::getCursorKey(){
    return kc::strprintf(cursor_format,status_,queue_id_,priority_,action_,doc_type_,doc_id_);
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
    // registry_->getQueueManager->insertCommand(DropDocument,shared_from_this());
    return false;
  }
  
  bool Command::addAssociation(){
    DocumentPtr doc=registry_->getDocumentManager()->getDocument(getDocType(),getDocId());
    registry_->getDocumentManager()->associateDocument(doc);
    return true;
  }

  bool Command::addAssociations(){
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
  
  class QueueManager{
  private:
    Registry* registry_;

  public:
    QueueManager(Registry* registry);
    ~QueueManager();
    CommandPtr insertCommand(const CommandAction action,CommandPtr source);
    CommandPtr createCommand(const CommandAction action,const uint32_t doc_type,const uint32_t doc_id,const string& payload);
    CommandPtr getQueuedCommand();
    size_t processQueue();
    void fillDictionary(TemplateDictionary* dict,const string& cursor);

  private:
    CommandPtr getCommand(const string& key,const string& value);
    void debug();
    DISALLOW_COPY_AND_ASSIGN(QueueManager);
  };
  
  QueueManager::QueueManager(Registry* registry):
  registry_(registry){}
  
  QueueManager::~QueueManager(){}
  
  CommandPtr QueueManager::createCommand(const CommandAction action,const uint32_t doc_type,const uint32_t doc_id,const string& payload){
    uint64_t queue_id=registry_->getMiscDB()->increment("QueueCounter",1);
    uint64_t payload_id=registry_->getMiscDB()->increment("PayloadCounter",1);
    return CommandPtr(new Command(registry_,action,queue_id,payload_id,doc_type,doc_id,payload));
  }
  
  CommandPtr QueueManager::insertCommand(const CommandAction action,CommandPtr source){
    uint64_t payload_id=registry_->getMiscDB()->increment("PayloadCounter",1);
    return CommandPtr(new Command(registry_,action,source->getQueueId(),payload_id,source->getDocType(),source->getDocId(),""));
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
      if ((not command->execute())||((previousAction!=NullAction)&&(previousAction!=command->getAction()))){
        registry_->getPostings()->wait();
      }else{
        count++;
      }
      // debug();
      previousAction=command->getAction();
      command = getQueuedCommand();
    }
    registry_->getPostings()->wait();
    return count;
  }
  
  void QueueManager::fillDictionary(TemplateDictionary* dict,const string& cursor){
    string cursor_key=(cursor=="")?toString(Queued):cursor;
    TemplateDictionary* pager_dict=dict->AddIncludeDictionary("PAGING");
    pager_dict->SetFilename(PAGING);
    kc::PolyDB::Cursor* queue_cursor=registry_->getQueueDB()->cursor();
    size_t count;
    vector<string> keys;
    string key;
    string value;
    CommandPtr command;
    if(queue_cursor->jump() && queue_cursor->get(&key,&value)){
      command = getCommand(key,value);
      pager_dict->SetValueAndShowSection("PAGE",command->getCursorKey(),"FIRST");
    }
    if(queue_cursor->jump_back() && queue_cursor->get(&key,&value)){
      command = getCommand(key,value);
      pager_dict->SetValueAndShowSection("PAGE",command->getCursorKey(),"LAST");
    }
    if (registry_->getQueueDB()->match_prefix(cursor_key,&keys,1)==1){
      key=keys[0];
      queue_cursor->jump(key);
      count=registry_->getPageSize();
      while(count--){
        queue_cursor->step_back();
      }
      if(queue_cursor->get(&key,&value)){
        command = getCommand(key,value);
        pager_dict->SetValueAndShowSection("PAGE",command->getCursorKey(),"PREVIOUS");
      }
    }else{
      queue_cursor->jump();
    }
    count=registry_->getPageSize();
    while(queue_cursor->get(&key,&value,true) && count>0){
      command = getCommand(key,value);
      command->fillDictionary(dict);
      count--;
    }
    if(queue_cursor->get(&key,&value)){
      command = getCommand(key,value);
      pager_dict->SetValueAndShowSection("PAGE",command->getCursorKey(),"NEXT");
    }
    delete queue_cursor;
  }
  
  void QueueManager::debug(){
    string key;
    string value;
    PolyDB::Cursor* cursor=registry_->getQueueDB()->cursor();
    cursor->jump();
    while(cursor->get(&key,&value,true)){
      cout << getCommand(key,value)->getKey() << endl;
    }
    cout << "--------------------------------------------------" <<endl;
    delete cursor;
  }
}

TEST_F(CommandTest,DictionaryTest){
  QueueManager* queueManager=new QueueManager(&registry_);
  for (size_t i=1;i<=200;i++){
    queueManager->createCommand(AddDocument,1,i,"text=This+is+a+test");
  }
  EXPECT_EQ(200U,queueManager->processQueue());
  for (size_t i=201;i<=400;i++){
    queueManager->createCommand(AddDocument,1,i,"text=This+is+a+test");
  }
  TemplateDictionary dict("test");
  dict.SetFilename(QUEUE_PAGE);
  queueManager->fillDictionary(&dict,"");
  dict.Dump();
}

TEST_F(CommandTest,PersistenceTest){
  QueueManager* queueManager=new QueueManager(&registry_);
  CommandPtr addDoc = queueManager->createCommand(AddDocument,1,1,"text=This+is+a+test");
  registry_.getPostings()->wait();
  EXPECT_STREQ("text=This+is+a+test",addDoc->getPayload().c_str());
  EXPECT_EQ(1,addDoc->getDocType());
  EXPECT_EQ(1,addDoc->getDocId());
  EXPECT_EQ(Queued,addDoc->getStatus());
  EXPECT_EQ(1U,registry_.getQueueDB()->count());
  EXPECT_EQ(1U,registry_.getPayloadDB()->count());
  EXPECT_STREQ("4:00000000000000000001:002:1:0000000001:0000000001",addDoc->getKey().c_str());
  CommandPtr savedCommand = queueManager->getQueuedCommand();
  EXPECT_TRUE(savedCommand);
  EXPECT_STREQ("4:00000000000000000001:002:1:0000000001:0000000001",savedCommand->getKey().c_str());
  EXPECT_STREQ("text=This+is+a+test",savedCommand->getPayload().c_str());
  EXPECT_EQ(1,savedCommand->getDocType());
  EXPECT_EQ(1,savedCommand->getDocId());
  EXPECT_EQ(Queued,savedCommand->getStatus());
  EXPECT_TRUE(savedCommand->execute());
  EXPECT_EQ(Active,savedCommand->getStatus());
  EXPECT_STREQ("3:00000000000000000001:002:1:0000000001:0000000001",savedCommand->getKey().c_str());
}

TEST_F(CommandTest,QueueTest){
  QueueManager* queueManager=new QueueManager(&registry_);
  CommandPtr addDoc1 = queueManager->createCommand(AddDocument,1,1,"text=This+is+a+test");
  CommandPtr addDoc2 = queueManager->createCommand(AddDocument,1,2,"text=This+is+a+test");
  CommandPtr addDoc3 = queueManager->createCommand(AddDocument,1,3,"text=This+is+a+test");
  EXPECT_EQ(3U,registry_.getQueueDB()->count());
  EXPECT_EQ(3U,registry_.getPayloadDB()->count());
  EXPECT_EQ(3U,queueManager->processQueue());
  EXPECT_EQ(3U,registry_.getDocumentDB()->count());
  EXPECT_EQ(0U,registry_.getAssociationDB()->count());
  CommandPtr associateDoc1 = queueManager->createCommand(AddAssociation,1,1,"");
  CommandPtr associateDoc2 = queueManager->createCommand(AddAssociation,1,2,"");
  CommandPtr associateDoc3 = queueManager->createCommand(AddAssociation,1,3,"");
  EXPECT_EQ(3U,queueManager->processQueue());
  EXPECT_EQ(6U,registry_.getQueueDB()->count());
  EXPECT_EQ(6U,registry_.getPayloadDB()->count());
  EXPECT_EQ(3U,registry_.getDocumentDB()->count());
  EXPECT_EQ(6U,registry_.getAssociationDB()->count());
  CommandPtr dropDoc1 = queueManager->createCommand(DropDocument,1,1,"");
  CommandPtr dropDoc2 = queueManager->createCommand(DropDocument,1,2,"");
  CommandPtr dropDoc3 = queueManager->createCommand(DropDocument,1,3,"");
  EXPECT_EQ(3U,queueManager->processQueue());
  EXPECT_EQ(9U,registry_.getQueueDB()->count());
  EXPECT_EQ(9U,registry_.getPayloadDB()->count());
  EXPECT_EQ(0U,registry_.getDocumentDB()->count());
  EXPECT_EQ(0U,registry_.getAssociationDB()->count());
}

TEST_F(CommandTest,DocumentTest){
  QueueManager* queueManager=new QueueManager(&registry_);
  CommandPtr addDoc = queueManager->createCommand(AddDocument,1,1,"text=This+is+a+test");
  EXPECT_TRUE(addDoc->execute());
  registry_.getPostings()->wait();
  EXPECT_EQ(11U,registry_.getPostings()->getHashCount());
  EXPECT_EQ(1U,registry_.getDocumentDB()->count());
  EXPECT_EQ(1U,registry_.getMetaDB()->count());
  CommandPtr dropDoc = queueManager->createCommand(DropDocument,1,1,"");
  EXPECT_TRUE(dropDoc->execute());
  registry_.getPostings()->wait();
  // TODO Implement this!
  // EXPECT_EQ(0U,registry_.getPostings()->getHashCount());
  EXPECT_EQ(0U,registry_.getDocumentDB()->count());
  EXPECT_EQ(0U,registry_.getMetaDB()->count());
}

TEST_F(CommandTest,AssociationTest){
  QueueManager* queueManager=new QueueManager(&registry_);
  CommandPtr addDoc1 = queueManager->createCommand(AddDocument,1,1,"text=This+is+a+test");
  EXPECT_TRUE(addDoc1->execute());
  CommandPtr addDoc2 = queueManager->createCommand(AddDocument,1,2,"text=This+is+a+test");
  EXPECT_TRUE(addDoc2->execute());
  registry_.getPostings()->wait();
  CommandPtr associateDoc1 = queueManager->createCommand(AddAssociation,1,1,"");
  EXPECT_TRUE(associateDoc1->execute());
  EXPECT_EQ(2U,registry_.getAssociationDB()->count());
  CommandPtr associateDoc2 = queueManager->createCommand(AddAssociation,1,2,"");
  EXPECT_TRUE(associateDoc2->execute());
  EXPECT_EQ(2U,registry_.getAssociationDB()->count());
  CommandPtr dropDoc1 = queueManager->createCommand(DropDocument,1,1,"");
  EXPECT_TRUE(dropDoc1->execute());
  EXPECT_EQ(1U,registry_.getDocumentDB()->count());
  EXPECT_EQ(0U,registry_.getAssociationDB()->count());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}