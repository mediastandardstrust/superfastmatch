#include <document.h>
#include <mock_registry.h>
#include <kcprotodb.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;
using namespace superfastmatch;
using namespace kyotocabinet;

TEST(DocumentTest,ConstructorTest){
  PolyDB* hashesDB = new PolyDB();
  PolyDB* documentDB = new PolyDB();
  hashesDB->open();
  documentDB->open();
  MockRegistry registry;
  EXPECT_CALL(registry,getWindowSize())
    .WillRepeatedly(Return(4));
  EXPECT_CALL(registry,getHashesDB())
    .WillRepeatedly(Return(hashesDB));
  EXPECT_CALL(registry,getDocumentDB())
    .WillRepeatedly(Return(hashesDB));
  Document* doc1 = new Document(1,1,"text=This+is+a+test&title=Also+a+test&filename=test.txt",&registry);
  Document* doc2 = new Document(1,2,"text=Another+test&title=Also+a+test&filename=test2.txt",&registry);
  EXPECT_EQ(14U,doc1->text().size());
  EXPECT_STREQ("This is a test",doc1->text().c_str());
  EXPECT_STREQ("this is a test",doc1->getCleanText().c_str());
  EXPECT_STREQ("Another test",doc2->text().c_str());
  EXPECT_STREQ("another test",doc2->getCleanText().c_str());
  EXPECT_EQ(11U,doc1->title().size());
  EXPECT_STREQ("Also a test",doc1->title().c_str());
  EXPECT_EQ(8U,doc1->content()["filename"].size());
  EXPECT_STREQ("test.txt",doc1->content()["filename"].c_str());
  doc1->save();
  delete doc1;
  delete doc2;
  hashesDB->close();
  documentDB->close();
  delete hashesDB;
  delete documentDB;
}

TEST(DocumentTest, CleanTextTest){
  MockRegistry registry;
  Document* doc = new Document(1,1,"text=This+a+test+with+*+*++***^$$#@@<>lots+of+whitespace",&registry);
  EXPECT_EQ(doc->text().size(),doc->getCleanText().size());
  EXPECT_EQ("this a test with                 lots of whitespace",doc->getCleanText());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
