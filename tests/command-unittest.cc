#include <document.h>
#include <posting.h>
#include <mock_registry.h>
#include <kcprotodb.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;
using namespace superfastmatch;
using namespace kyotocabinet;

class CommandTest :public ::testing::Test{
protected:
  PolyDB* documentDB_;
  PolyDB* metaDB_;
  PolyDB* queueDB_;
  PolyDB* miscDB_;
  DocumentManager* documentManager_;
  Posting* postings_;
  MockRegistry registry_;
  Logger* logger_;
  
  virtual void SetUp(){
    documentDB_ = new PolyDB();
    metaDB_ = new PolyDB();
    queueDB_ = new PolyDB();
    miscDB_ = new PolyDB();
    documentDB_->open();
    metaDB_->open();
    queueDB_->open();
    miscDB_->open();
    EXPECT_CALL(registry_,getWindowSize())
      .WillRepeatedly(Return(4));
    EXPECT_CALL(registry_,getSlotCount())
      .WillRepeatedly(Return(4));
    EXPECT_CALL(registry_,getWhiteSpaceThreshold())
      .WillRepeatedly(Return(4));
    EXPECT_CALL(registry_,getWhiteSpaceHash(false))
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
      
    EXPECT_CALL(registry_,getMetaDB())
      .WillRepeatedly(Return(metaDB_));
    EXPECT_CALL(registry_,getMiscDB())
      .WillRepeatedly(Return(miscDB_));
    EXPECT_CALL(registry_,getDocumentDB())
      .WillRepeatedly(Return(documentDB_));

    documentManager_ = new DocumentManager(&registry_);
    postings_ = new Posting(&registry_);
    logger_ = new Logger();
    EXPECT_CALL(registry_,getDocumentManager())
      .WillRepeatedly(Return(documentManager_));
    EXPECT_CALL(registry_,getPostings())
      .WillRepeatedly(Return(postings_));
    EXPECT_CALL(registry_,getLogger())
      .WillRepeatedly(Return(logger_));


  }
  
  virtual void TearDown(){
    metaDB_->close();
    documentDB_->close();
    queueDB_->close();
    delete metaDB_;
    delete documentDB_;
    delete queueDB_;
    delete miscDB_;
    delete documentManager_;
    delete postings_;
    delete logger_;
  }
};

// Testing!
namespace superfastmatch{
  enum CommandAction{
    AddDocument,
    AddAssociation,
    AddAssociations,
    DropDocument,
    DropAssociation
  };

  enum CommandStatus{
    Queued,
    Active,
    Finished,
    Failed
  };

  // Stick in common.h
  class Command;
  typedef std::tr1::shared_ptr<Command> CommandPtr;

  // Stick in command.h
  class Command{
  public:
    typedef bool(Command::*Action)();
    typedef map<CommandAction,pair<Action,string> > action_map;
  private:
    Registry* registry_;
    uint64_t queue_id_;
    uint32_t priority_;
    CommandStatus status_;
    CommandAction action_;
    uint32_t doc_type_;
    uint32_t doc_id_;
    string* payload_;

    static const char* key_format;
    static const action_map actions;
    static const map<CommandStatus,string> statuses;
    
    bool addDocument();
    bool addAssociation();
    bool addAssociations();
    bool dropDocument();
    bool dropAssociation();

  public:
    ~Command();
    bool execute();
    void fill_list_dictionary(TemplateDictionary* dict);

    static CommandPtr create(Registry* registry,const CommandAction action,const uint32_t doc_type,const uint32_t doc_id,const string& payload);
    static CommandPtr get(Registry* registry,const string& key);
    
  private:
    Command(Registry* registry,const CommandAction action,const uint32_t doc_type,const uint32_t doc_id,const string& payload);
    DISALLOW_COPY_AND_ASSIGN(Command);
  };
  
  const Command::action_map Command::actions=create_map<CommandAction,pair<Command::Action,string> >(AddDocument,make_pair(&Command::addDocument,"Add Document"))\
                                                                                                    (AddAssociation,make_pair(&Command::addAssociation,"Add Association"))\
                                                                                                    (AddAssociations,make_pair(&Command::addAssociations,"Add Associations"))\
                                                                                                    (DropDocument,make_pair(&Command::dropDocument,"Drop Document"))\
                                                                                                    (DropAssociation,make_pair(&Command::dropAssociation,"Drop Association"));
  
  Command::Command(Registry* registry,const CommandAction action,const uint32_t doc_type,const uint32_t doc_id,const string& payload):
  registry_(registry),
  action_(action),
  doc_type_(doc_type),
  doc_id_(doc_id),
  payload_(new string(payload))
  {
    queue_id_=registry->getMiscDB()->increment("QueueCounter",1);
  }
  
  Command::~Command(){
    delete payload_;
  }
  
  CommandPtr Command::create(Registry* registry,const CommandAction action,const uint32_t doc_type,const uint32_t doc_id,const string& payload){
    return CommandPtr(new Command(registry,action,doc_type,doc_id,payload));
  }
  
  bool Command::execute(){
    action_map::const_iterator action=actions.find(action_);
    if (action!=actions.end()){
      return (this->*action->second.first)();
    }
    return false;
  }
  
  bool Command::addDocument(){
    DocumentPtr doc = registry_->getDocumentManager()->createPermanentDocument(doc_type_,doc_id_,*payload_);
    registry_->getPostings()->addDocument(doc);
    return true;
  }
  
  bool Command::addAssociation(){
    return true;
  }

  bool Command::addAssociations(){
    return true;
  }
  
  bool Command::dropDocument(){
    return true;
  }
  
  bool Command::dropAssociation(){
    return true;
  }
}


TEST_F(CommandTest,PolymorphicTest){
  CommandPtr addDoc = Command::create(&registry_,AddDocument,1,1,"text=This+is+a+test");
  EXPECT_TRUE(addDoc->execute());
  registry_.getPostings()->wait();
  EXPECT_EQ(11U,registry_.getPostings()->getHashCount());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}