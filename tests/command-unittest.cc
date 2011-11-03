#include <tests.h>

typedef BaseTest CommandTest;

TEST_F(CommandTest,DictionaryTest){
  for (size_t i=1;i<=200;i++){
    registry_.getQueueManager()->createCommand(AddDocument,1,i,"text=This+is+a+test");
  }
  EXPECT_EQ(200U,registry_.getQueueManager()->processQueue());
  for (size_t i=201;i<=400;i++){
    registry_.getQueueManager()->createCommand(AddDocument,1,i,"text=This+is+a+test");
  }
  // TemplateDictionary dict("test");
  // dict.SetFilename(QUEUE_PAGE);
  // registry_.getQueueManager()->fillDictionary(&dict);
  // string data = dict.DumpToString();
  // TODO write a decent test!
  // dict.Dump();
}

TEST_F(CommandTest,PersistenceTest){
  CommandPtr addDoc = registry_.getQueueManager()->createCommand(AddDocument,1,1,"text=This+is+a+test");
  registry_.getPostings()->finishTasks();
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
  EXPECT_EQ(3U,registry_.getPayloadDB()->count());
  EXPECT_EQ(3U,registry_.getQueueManager()->processQueue());
  EXPECT_EQ(6U,registry_.getQueueDB()->count());
  EXPECT_EQ(0U,registry_.getPayloadDB()->count());
  EXPECT_EQ(3U,registry_.getDocumentDB()->count());
  EXPECT_EQ(6U,registry_.getAssociationDB()->count());
  CommandPtr dropDoc1 = registry_.getQueueManager()->createCommand(DropDocument,1,1,"");
  CommandPtr dropDoc2 = registry_.getQueueManager()->createCommand(DropDocument,1,2,"");
  CommandPtr dropDoc3 = registry_.getQueueManager()->createCommand(DropDocument,1,3,"");
  EXPECT_EQ(3U,registry_.getPayloadDB()->count());
  EXPECT_EQ(3U,registry_.getQueueManager()->processQueue());
  EXPECT_EQ(9U,registry_.getQueueDB()->count());
  EXPECT_EQ(0U,registry_.getPayloadDB()->count());
  EXPECT_EQ(0U,registry_.getDocumentDB()->count());
  EXPECT_EQ(0U,registry_.getAssociationDB()->count());
}

TEST_F(CommandTest,DocumentTest){
  CommandPtr addDoc = registry_.getQueueManager()->createCommand(AddDocument,1,1,"text=This+is+a+test");
  EXPECT_TRUE(addDoc->execute());
  registry_.getPostings()->finishTasks();
  EXPECT_EQ(11U,registry_.getPostings()->getHashCount());
  EXPECT_EQ(1U,registry_.getDocumentDB()->count());
  EXPECT_EQ(3U,registry_.getMetaDB()->count());
  CommandPtr dropDoc = registry_.getQueueManager()->createCommand(DropDocument,1,1,"");
  EXPECT_TRUE(dropDoc->execute());
  registry_.getPostings()->finishTasks();
  EXPECT_EQ(0U,registry_.getDocumentDB()->count());
  EXPECT_EQ(0U,registry_.getMetaDB()->count());
}

TEST_F(CommandTest,AssociationTest){
  CommandPtr addDoc1 = registry_.getQueueManager()->createCommand(AddDocument,1,1,"text=This+is+a+test");
  EXPECT_TRUE(addDoc1->execute());
  CommandPtr addDoc2 = registry_.getQueueManager()->createCommand(AddDocument,1,2,"text=This+is+a+test");
  EXPECT_TRUE(addDoc2->execute());
  registry_.getPostings()->finishTasks();
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