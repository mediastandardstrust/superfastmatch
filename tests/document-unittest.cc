#include <document.h>
#include <mock_registry.h>
#include <kcprotodb.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;
using namespace superfastmatch;
using namespace kyotocabinet;

TEST(DocumentTest,ConstructorTest){
  BasicDB* hashesDB = new ProtoTreeDB();
  MockRegistry registry;
  EXPECT_CALL(registry,getWindowSize())
    .WillRepeatedly(Return(4));
  EXPECT_CALL(registry,getHashesDB())
    .WillRepeatedly(Return(hashesDB));
  Document* doc = new Document(1,1,"text=this+is+a+test&title=Also+a+test&filename=test.txt",&registry);
  EXPECT_EQ(14U,doc->text().size());
  EXPECT_STREQ("this is a test",doc->text().c_str());
  EXPECT_EQ(11U,doc->title().size());
  EXPECT_STREQ("Also a test",doc->title().c_str());
  EXPECT_EQ(8U,doc->content()["filename"].size());
  EXPECT_STREQ("test.txt",doc->content()["filename"].c_str());
  doc->save();
  delete doc;
}

int main(int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}