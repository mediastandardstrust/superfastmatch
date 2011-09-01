#include <stdexcept>
#include <document.h>
#include <mock_registry.h>
#include <kcprotodb.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;
using namespace superfastmatch;
using namespace kyotocabinet;

class DocumentTest :public ::testing::Test{
protected:
  PolyDB* documentDB_;
  PolyDB* metaDB_;
  DocumentManager* documentManager_;
  MockRegistry registry_;
  
  virtual void SetUp(){
    documentDB_ = new PolyDB();
    metaDB_ = new PolyDB();
    documentManager_ = new DocumentManager(&registry_);
    documentDB_->open();
    metaDB_->open();
    EXPECT_CALL(registry_,getWindowSize())
      .WillRepeatedly(Return(4));
    EXPECT_CALL(registry_,getDocumentDB())
      .WillRepeatedly(Return(documentDB_));
    EXPECT_CALL(registry_,getDocumentManager())
      .WillRepeatedly(Return(documentManager_));
    EXPECT_CALL(registry_,getMetaDB())
      .WillRepeatedly(Return(metaDB_));
    EXPECT_CALL(registry_,getWhiteSpaceThreshold())
      .WillRepeatedly(Return(4));
    EXPECT_CALL(registry_,getWhiteSpaceHash(false))
      .WillRepeatedly(Return(0));  
  }
  
  virtual void TearDown(){
    metaDB_->close();
    documentDB_->close();
    delete metaDB_;
    delete documentDB_;
    delete documentManager_;
  }
};

typedef DocumentTest DocumentDeathTest;

TEST_F(DocumentTest,ConstructorTest){
  DocumentPtr doc1=registry_.getDocumentManager()->createPermanentDocument(1,1,"text=This+is+a+test&title=Also+a+test&filename=test.txt");
  DocumentPtr doc2=registry_.getDocumentManager()->createPermanentDocument(1,2,"text=Another+test&title=Also+a+test&filename=test2.txt");
  EXPECT_EQ(14U,doc1->getText().size());
  EXPECT_STREQ("This is a test",doc1->getText().c_str());
  EXPECT_STREQ("this is a test",doc1->getCleanText().c_str());
  EXPECT_STREQ("Another test",doc2->getText().c_str());
  EXPECT_STREQ("another test",doc2->getCleanText().c_str());
  EXPECT_EQ(11U,doc1->getMeta("title").size());
  EXPECT_STREQ("Also a test",doc1->getMeta("title").c_str());
  EXPECT_EQ(8U,doc1->getMeta("filename").size());
  EXPECT_STREQ("test.txt",doc1->getMeta("filename").c_str());
  DocumentPtr doc3 =registry_.getDocumentManager()->getDocument(1,1);
  EXPECT_STREQ("test.txt",doc3->getMeta("filename").c_str());
  EXPECT_EQ(doc3->getText().size()-registry_.getWindowSize()+1,doc3->getHashes().size());
  EXPECT_STREQ("Also a test",doc3->getMeta("title").c_str());
  EXPECT_STREQ("This is a test",doc3->getText().c_str());
}

TEST_F(DocumentTest, CleanTextTest){
  DocumentPtr doc=registry_.getDocumentManager()->createTemporaryDocument("text=This+a+test+with+*+*++***^$$#@@<>lots+of+whitespace");
  EXPECT_EQ(doc->getText().size(),doc->getCleanText().size());
  EXPECT_EQ("this a test with                 lots of whitespace",doc->getCleanText());
}

TEST_F(DocumentTest, MetaTest){
  string value;
  vector<string> keys;
  DocumentPtr doc1=registry_.getDocumentManager()->createPermanentDocument(1,1,"text=some+short+text+with_metadata&title=Title&filename=test.txt");
  EXPECT_STREQ("Title",doc1->getMeta("title").c_str());
  EXPECT_STREQ("test.txt",doc1->getMeta("filename").c_str());
  EXPECT_TRUE(doc1->setMeta("count","4"));
  EXPECT_STREQ("4",doc1->getMeta("count").c_str());
  EXPECT_STRNE("",doc1->getMeta("characters").c_str());
  EXPECT_STREQ("",doc1->getMeta("non-existent").c_str());
  EXPECT_TRUE(doc1->getMetaKeys(keys));
  EXPECT_EQ(4U,keys.size());
  DocumentPtr doc2=registry_.getDocumentManager()->getDocument(1,1,DocumentManager::META);
  EXPECT_STREQ("4",doc2->getMeta("count").c_str());
  EXPECT_STREQ("Title",doc2->getMeta("title").c_str());
  EXPECT_STREQ("test.txt",doc2->getMeta("filename").c_str());
  EXPECT_TRUE(doc1->getMetaKeys(keys));
  EXPECT_EQ(4U,keys.size());
}

TEST_F(DocumentTest,ManagerTest){
  DocumentPtr tempDoc=registry_.getDocumentManager()->createTemporaryDocument("text=some+short+text+with_metadata&title=Title");
  DocumentPtr permDoc=registry_.getDocumentManager()->createPermanentDocument(1,1,"text=some+short+text+with_metadata&title=Title");
  DocumentPtr savedDoc=registry_.getDocumentManager()->getDocument(1,1);
  DocumentPtr savedDocFromKey=registry_.getDocumentManager()->getDocument(permDoc->getKey());
  EXPECT_STREQ("Title",tempDoc->getMeta("title").c_str());
  EXPECT_STREQ("Title",permDoc->getMeta("title").c_str());
  EXPECT_STREQ("Title",savedDoc->getMeta("title").c_str());
  EXPECT_STREQ("Title",savedDocFromKey->getMeta("title").c_str());
  EXPECT_EQ(0U,tempDoc->doctype());
  EXPECT_EQ(0U,tempDoc->docid());
  EXPECT_EQ(1U,permDoc->doctype());
  EXPECT_EQ(1U,permDoc->docid());
  EXPECT_EQ(1U,savedDoc->doctype());
  EXPECT_EQ(1U,savedDoc->docid());
  EXPECT_EQ(1U,savedDocFromKey->doctype());
  EXPECT_EQ(1U,savedDocFromKey->docid());
  EXPECT_NE(0U,tempDoc->getHashes().size());
  EXPECT_NE(0U,permDoc->getHashes().size());
  EXPECT_NE(0U,savedDoc->getHashes().size());
  EXPECT_NE(0U,savedDocFromKey->getHashes().size());
}

TEST_F(DocumentDeathTest,RemovalTest){
  DocumentPtr permDoc=registry_.getDocumentManager()->createPermanentDocument(1,1,"text=some+short+text+with_metadata&title=Title");
  EXPECT_EQ(1U,permDoc->doctype());
  EXPECT_EQ(1U,permDoc->docid());
  EXPECT_STREQ("Title",permDoc->getMeta("title").c_str());
  EXPECT_TRUE(registry_.getDocumentManager()->removePermanentDocument(permDoc));
  EXPECT_DEATH(registry_.getDocumentManager()->getDocument(1,1),"^Assertion failed");
}

TEST_F(DocumentTest,DuplicatePermanentTest){
  DocumentPtr permDoc=registry_.getDocumentManager()->createPermanentDocument(1,1,"text=some+short+text+with_metadata&title=Title");
  EXPECT_EQ(1U,permDoc->doctype());
  EXPECT_EQ(1U,permDoc->docid());
  EXPECT_STREQ("Title",permDoc->getMeta("title").c_str());
  EXPECT_FALSE(registry_.getDocumentManager()->createPermanentDocument(1,1,"text=some+short+text+with_metadata&title=Title"));
}

TEST_F(DocumentTest,InitTest){
  DocumentPtr tempDoc=registry_.getDocumentManager()->createTemporaryDocument("text=Some+short+text+with+metadata&title=Title",NULL);
  EXPECT_STREQ("Some short text with metadata",tempDoc->getText().c_str());
  EXPECT_STREQ("Title",tempDoc->getMeta("title").c_str());
  ASSERT_THROW(tempDoc->getCleanText(),std::runtime_error);
  ASSERT_THROW(tempDoc->getHashes(),std::runtime_error);
  ASSERT_THROW(tempDoc->getBloom(),std::runtime_error);
  tempDoc=registry_.getDocumentManager()->createTemporaryDocument("text=Some+short+text+with+metadata&title=Title",DocumentManager::CLEAN_TEXT);
  EXPECT_STREQ("Some short text with metadata",tempDoc->getText().c_str());
  EXPECT_STREQ("Title",tempDoc->getMeta("title").c_str());
  EXPECT_STREQ("some short text with metadata",tempDoc->getCleanText().c_str());
  ASSERT_THROW(tempDoc->getHashes(),std::runtime_error);
  ASSERT_THROW(tempDoc->getBloom(),std::runtime_error);
  tempDoc=registry_.getDocumentManager()->createTemporaryDocument("text=Some+short+text+with+metadata&title=Title",DocumentManager::CLEAN_TEXT|DocumentManager::HASHES);
  EXPECT_STREQ("Some short text with metadata",tempDoc->getText().c_str());
  EXPECT_STREQ("Title",tempDoc->getMeta("title").c_str());
  EXPECT_STREQ("some short text with metadata",tempDoc->getCleanText().c_str());
  EXPECT_GT(tempDoc->getHashes().size(),0U);
  ASSERT_THROW(tempDoc->getBloom(),std::runtime_error);
  tempDoc=registry_.getDocumentManager()->createTemporaryDocument("text=Some+short+text+with+metadata&title=Title",DocumentManager::CLEAN_TEXT|DocumentManager::HASHES|DocumentManager::BLOOM);
  EXPECT_STREQ("Some short text with metadata",tempDoc->getText().c_str());
  EXPECT_STREQ("Title",tempDoc->getMeta("title").c_str());
  EXPECT_STREQ("some short text with metadata",tempDoc->getCleanText().c_str());
  EXPECT_GT(tempDoc->getHashes().size(),0U);
  EXPECT_GT(tempDoc->getBloom().size(),0U);
  ASSERT_THROW(registry_.getDocumentManager()->createTemporaryDocument("text=Some+short+text+with+metadata&title=Title",DocumentManager::BLOOM),std::runtime_error);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
