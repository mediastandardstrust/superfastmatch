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
  MockRegistry registry_;
  
  virtual void SetUp(){
    documentDB_ = new PolyDB();
    metaDB_ = new PolyDB();
    documentDB_->open();
    metaDB_->open();
    EXPECT_CALL(registry_,getWindowSize())
      .WillRepeatedly(Return(4));
    EXPECT_CALL(registry_,getDocumentDB())
      .WillRepeatedly(Return(documentDB_));
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
  }
};

TEST_F(DocumentTest,ConstructorTest){

  Document* doc1 = new Document(1,1,"text=This+is+a+test&title=Also+a+test&filename=test.txt",&registry_);
  Document* doc2 = new Document(1,2,"text=Another+test&title=Also+a+test&filename=test2.txt",&registry_);
  EXPECT_EQ(14U,doc1->getText().size());
  EXPECT_STREQ("This is a test",doc1->getText().c_str());
  EXPECT_STREQ("this is a test",doc1->getCleanText().c_str());
  EXPECT_STREQ("Another test",doc2->getText().c_str());
  EXPECT_STREQ("another test",doc2->getCleanText().c_str());
  EXPECT_EQ(11U,doc1->getMeta("title").size());
  EXPECT_STREQ("Also a test",doc1->getMeta("title").c_str());
  EXPECT_EQ(8U,doc1->getMeta("filename").size());
  EXPECT_STREQ("test.txt",doc1->getMeta("filename").c_str());
  doc1->save();
  Document* doc3 = new Document(1,1,&registry_);
  EXPECT_STREQ("test.txt",doc3->getMeta("filename").c_str());
  EXPECT_EQ(doc3->getText().size()-4,doc3->hashes().size());
  EXPECT_STREQ("Also a test",doc3->getMeta("title").c_str());
  EXPECT_STREQ("This is a test",doc3->getText().c_str());
  delete doc1;
  delete doc2;
  delete doc3;
}

TEST_F(DocumentTest, CleanTextTest){
  Document* doc = new Document(1,1,"text=This+a+test+with+*+*++***^$$#@@<>lots+of+whitespace",&registry_);
  EXPECT_EQ(doc->getText().size(),doc->getCleanText().size());
  EXPECT_EQ("this a test with                 lots of whitespace",doc->getCleanText());
}

TEST_F(DocumentTest, MetaTest){
  string value;
  vector<string> keys;
  Document* doc1 = new Document(1,1,"text=some+short+text+with_metadata&title=Title&filename=test.txt",&registry_);
  EXPECT_STREQ("Title",doc1->getMeta("title").c_str());
  EXPECT_STREQ("test.txt",doc1->getMeta("filename").c_str());
  EXPECT_TRUE(doc1->setMeta("count","4"));
  EXPECT_STREQ("4",doc1->getMeta("count").c_str());
  EXPECT_STRNE("",doc1->getMeta("characters").c_str());
  EXPECT_STREQ("",doc1->getMeta("non-existent").c_str());
  EXPECT_TRUE(doc1->getMetaKeys(keys));
  EXPECT_EQ(4U,keys.size());
  doc1->save();
  Document* doc2 = new Document(1,1,&registry_);
  EXPECT_STREQ("4",doc2->getMeta("count").c_str());
  EXPECT_STREQ("Title",doc2->getMeta("title").c_str());
  EXPECT_STREQ("test.txt",doc2->getMeta("filename").c_str());
  EXPECT_TRUE(doc1->getMetaKeys(keys));
  EXPECT_EQ(4U,keys.size());
  delete doc1;
  delete doc2;
}

TEST_F(DocumentTest,ManagerTest){
  
}

int main(int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
