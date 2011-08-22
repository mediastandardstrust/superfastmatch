#include <kcprotodb.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <association.h>
#include <document.h>
#include <mock_registry.h>

using namespace testing;
using namespace superfastmatch;
using namespace kyotocabinet;

TEST(AssociationTest,ConstructorTest){
  MockRegistry registry;
  PolyDB* associationDB = new PolyDB();
  associationDB->open();
  EXPECT_CALL(registry,getLogger())
    .WillRepeatedly(Return(new Logger()));
  EXPECT_CALL(registry,getWindowSize())
    .WillRepeatedly(Return(15));
  EXPECT_CALL(registry,getWhiteSpaceThreshold())
    .WillRepeatedly(Return(4));
  EXPECT_CALL(registry,getWhiteSpaceHash(false))
    .WillRepeatedly(Return(0));
  EXPECT_CALL(registry,getWhiteSpaceHash(true))
    .WillRepeatedly(Return(0));
  EXPECT_CALL(registry,getAssociationDB())
    .WillRepeatedly(Return(associationDB));
  Document* doc1 = new Document(1,1,"text=This+is+a+long+sentence+where+the+phrase+Always+Look+On+The+Bright+Side+Of+Life&title=Doc1",&registry);
  Document* doc2 = new Document(1,2,"text=Always+Look+On+The+Bright+Side+Of+Life+and+this+is+a+long+sentence&title=Doc2",&registry);
  EXPECT_STREQ("This is a long sentence where the phrase Always Look On The Bright Side Of Life",doc1->getText().c_str());
  EXPECT_STREQ("Always Look On The Bright Side Of Life and this is a long sentence",doc2->getText().c_str());
  Association* association = new Association(&registry,doc1,doc2);
  EXPECT_EQ(2U,association->getResultCount());
  EXPECT_EQ(38U,association->getLength(0));
  EXPECT_STREQ("Always Look On The Bright Side Of Life",association->getToResult(0).c_str());
  EXPECT_STREQ(association->getToResult(0).c_str(),association->getFromResult(0).c_str());
  EXPECT_EQ(23U,association->getLength(1));
  EXPECT_STREQ("This is a long sentence",association->getFromResult(1).c_str());
  EXPECT_STRCASEEQ(association->getToResult(1).c_str(),association->getFromResult(1).c_str());
  EXPECT_EQ(61U,association->getTotalLength());
  EXPECT_EQ(true,association->save());
  Association* association2 = new Association(&registry,doc1,doc2);
  EXPECT_EQ(2U,association2->getResultCount());  
  Association* association3 = new Association(&registry,doc2,doc1);
  EXPECT_EQ(2U,association3->getResultCount());
  EXPECT_STRCASEEQ(association2->getFromResult(1).c_str(),association3->getFromResult(1).c_str());
  EXPECT_EQ(2U,associationDB->count());
  delete association;
  delete association2;
  delete association3;
  delete doc1;
  delete doc2;
  associationDB->close();
  delete associationDB;
}

TEST(AssociationTest, WhitespaceTest){
  MockRegistry registry;
  PolyDB* associationDB = new PolyDB();
  associationDB->open();
  EXPECT_CALL(registry,getLogger())
    .WillRepeatedly(Return(new Logger()));
  EXPECT_CALL(registry,getWindowSize())
    .WillRepeatedly(Return(15));
  EXPECT_CALL(registry,getWhiteSpaceThreshold())
    .WillRepeatedly(Return(4));
  EXPECT_CALL(registry,getWhiteSpaceHash(false))
    .WillRepeatedly(Return(0));
  EXPECT_CALL(registry,getWhiteSpaceHash(true))
    .WillRepeatedly(Return(0));
  EXPECT_CALL(registry,getAssociationDB())
    .WillRepeatedly(Return(associationDB));
  Document* doc1 = new Document(1,1,"text=++++++++++++++++++++whitespace+test+with+a+long+sentence+*+*+*+*+*+*+*+*+*+*+*+*+",&registry);
  Document* doc2 = new Document(1,2,"text=++++++++++++++++++++whitespace+test+with+a+long+sentence+*+*+*+*+*+*+*+*+*+*+*+*+",&registry);
  Association* association = new Association(&registry,doc1,doc2);
  EXPECT_STREQ("   whitespace test with a long sentence * *",association->getToResult(0).c_str());
}

TEST(AssociationTest, EndingTest){
  MockRegistry registry;
  PolyDB* associationDB = new PolyDB();
  associationDB->open();
  EXPECT_CALL(registry,getLogger())
    .WillRepeatedly(Return(new Logger()));
  EXPECT_CALL(registry,getWindowSize())
    .WillRepeatedly(Return(15));
  EXPECT_CALL(registry,getWhiteSpaceThreshold())
    .WillRepeatedly(Return(4));
  EXPECT_CALL(registry,getWhiteSpaceHash(false))
    .WillRepeatedly(Return(0));
  EXPECT_CALL(registry,getWhiteSpaceHash(true))
    .WillRepeatedly(Return(0));
  EXPECT_CALL(registry,getAssociationDB())
    .WillRepeatedly(Return(associationDB));
  Document* doc1 = new Document(1,1,"text=This+is+Captain+Franklin",&registry);
  Document* doc2 = new Document(1,2,"text=This+is+Captain+Francis",&registry);
  Association* association = new Association(&registry,doc1,doc2);
  EXPECT_STREQ("This is Captain Fran",association->getToResult(0).c_str());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}