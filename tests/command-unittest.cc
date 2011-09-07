#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <document.h>
#include <posting.h>
#include <queue.h>
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
  QueueManager* queueManager_;
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
    queueManager_ = new QueueManager(&registry_);
    postings_ = new Posting(&registry_);
    logger_ = new Logger();
    EXPECT_CALL(registry_,getDocumentManager())
      .WillRepeatedly(Return(documentManager_));
    EXPECT_CALL(registry_,getAssociationManager())
      .WillRepeatedly(Return(associationManager_));
    EXPECT_CALL(registry_,getQueueManager())
      .WillRepeatedly(Return(queueManager_));
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
    delete queueManager_;
    delete postings_;
    delete logger_;
  }
};

TEST_F(CommandTest,DictionaryTest){
  for (size_t i=1;i<=200;i++){
    registry_.getQueueManager()->createCommand(AddDocument,1,i,"text=This+is+a+test");
  }
  EXPECT_EQ(200U,registry_.getQueueManager()->processQueue());
  for (size_t i=201;i<=400;i++){
    registry_.getQueueManager()->createCommand(AddDocument,1,i,"text=This+is+a+test");
  }
  TemplateDictionary dict("test");
  dict.SetFilename(QUEUE_PAGE);
  registry_.getQueueManager()->fillDictionary(&dict);
  // string data = dict.DumpToString();
  // TODO write a decent test!
  // dict.Dump();
}

TEST_F(CommandTest,PersistenceTest){
  CommandPtr addDoc = registry_.getQueueManager()->createCommand(AddDocument,1,1,"text=This+is+a+test");
  registry_.getPostings()->wait();
  EXPECT_STREQ("text=This+is+a+test",addDoc->getPayload().c_str());
  EXPECT_EQ(1U,addDoc->getDocType());
  EXPECT_EQ(1U,addDoc->getDocId());
  EXPECT_EQ(Queued,addDoc->getStatus());
  EXPECT_EQ(1U,registry_.getQueueDB()->count());
  EXPECT_EQ(1U,registry_.getPayloadDB()->count());
  EXPECT_STREQ("4:00000000000000000001:002:1:0000000001:0000000001",addDoc->getKey().c_str());
  CommandPtr savedCommand = registry_.getQueueManager()->getQueuedCommand();
  EXPECT_TRUE(savedCommand);
  EXPECT_STREQ("4:00000000000000000001:002:1:0000000001:0000000001",savedCommand->getKey().c_str());
  EXPECT_STREQ("text=This+is+a+test",savedCommand->getPayload().c_str());
  EXPECT_EQ(1U,savedCommand->getDocType());
  EXPECT_EQ(1U,savedCommand->getDocId());
  EXPECT_EQ(Queued,savedCommand->getStatus());
  EXPECT_TRUE(savedCommand->execute());
  EXPECT_EQ(Active,savedCommand->getStatus());
  EXPECT_STREQ("3:00000000000000000001:002:1:0000000001:0000000001",savedCommand->getKey().c_str());
}

TEST_F(CommandTest,QueueTest){
  CommandPtr addDoc1 = registry_.getQueueManager()->createCommand(AddDocument,1,1,"text=This+is+a+test");
  CommandPtr addDoc2 = registry_.getQueueManager()->createCommand(AddDocument,1,2,"text=This+is+a+test");
  CommandPtr addDoc3 = registry_.getQueueManager()->createCommand(AddDocument,1,3,"text=This+is+a+test");
  EXPECT_EQ(3U,registry_.getQueueDB()->count());
  EXPECT_EQ(3U,registry_.getPayloadDB()->count());
  EXPECT_EQ(3U,registry_.getQueueManager()->processQueue());
  EXPECT_EQ(3U,registry_.getDocumentDB()->count());
  EXPECT_EQ(0U,registry_.getAssociationDB()->count());
  CommandPtr associateDoc1 = registry_.getQueueManager()->createCommand(AddAssociation,1,1,"");
  CommandPtr associateDoc2 = registry_.getQueueManager()->createCommand(AddAssociation,1,2,"");
  CommandPtr associateDoc3 = registry_.getQueueManager()->createCommand(AddAssociation,1,3,"");
  EXPECT_EQ(3U,registry_.getQueueManager()->processQueue());
  EXPECT_EQ(6U,registry_.getQueueDB()->count());
  EXPECT_EQ(6U,registry_.getPayloadDB()->count());
  EXPECT_EQ(3U,registry_.getDocumentDB()->count());
  EXPECT_EQ(6U,registry_.getAssociationDB()->count());
  CommandPtr dropDoc1 = registry_.getQueueManager()->createCommand(DropDocument,1,1,"");
  CommandPtr dropDoc2 = registry_.getQueueManager()->createCommand(DropDocument,1,2,"");
  CommandPtr dropDoc3 = registry_.getQueueManager()->createCommand(DropDocument,1,3,"");
  EXPECT_EQ(3U,registry_.getQueueManager()->processQueue());
  EXPECT_EQ(9U,registry_.getQueueDB()->count());
  EXPECT_EQ(9U,registry_.getPayloadDB()->count());
  EXPECT_EQ(0U,registry_.getDocumentDB()->count());
  EXPECT_EQ(0U,registry_.getAssociationDB()->count());
}

TEST_F(CommandTest,DocumentTest){
  CommandPtr addDoc = registry_.getQueueManager()->createCommand(AddDocument,1,1,"text=This+is+a+test");
  EXPECT_TRUE(addDoc->execute());
  registry_.getPostings()->wait();
  EXPECT_EQ(11U,registry_.getPostings()->getHashCount());
  EXPECT_EQ(1U,registry_.getDocumentDB()->count());
  EXPECT_EQ(1U,registry_.getMetaDB()->count());
  CommandPtr dropDoc = registry_.getQueueManager()->createCommand(DropDocument,1,1,"");
  EXPECT_TRUE(dropDoc->execute());
  registry_.getPostings()->wait();
  // TODO Implement this!
  // EXPECT_EQ(0U,registry_.getPostings()->getHashCount());
  EXPECT_EQ(0U,registry_.getDocumentDB()->count());
  EXPECT_EQ(0U,registry_.getMetaDB()->count());
}

TEST_F(CommandTest,AssociationTest){
  CommandPtr addDoc1 = registry_.getQueueManager()->createCommand(AddDocument,1,1,"text=This+is+a+test");
  EXPECT_TRUE(addDoc1->execute());
  CommandPtr addDoc2 = registry_.getQueueManager()->createCommand(AddDocument,1,2,"text=This+is+a+test");
  EXPECT_TRUE(addDoc2->execute());
  registry_.getPostings()->wait();
  CommandPtr associateDoc1 = registry_.getQueueManager()->createCommand(AddAssociation,1,1,"");
  EXPECT_TRUE(associateDoc1->execute());
  EXPECT_EQ(2U,registry_.getAssociationDB()->count());
  CommandPtr associateDoc2 = registry_.getQueueManager()->createCommand(AddAssociation,1,2,"");
  EXPECT_TRUE(associateDoc2->execute());
  EXPECT_EQ(2U,registry_.getAssociationDB()->count());
  CommandPtr dropDoc1 = registry_.getQueueManager()->createCommand(DropDocument,1,1,"");
  EXPECT_TRUE(dropDoc1->execute());
  EXPECT_EQ(1U,registry_.getDocumentDB()->count());
  EXPECT_EQ(0U,registry_.getAssociationDB()->count());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}