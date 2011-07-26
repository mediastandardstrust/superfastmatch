#include <document.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;
using namespace superfastmatch;

TEST(PostLineTest,VarIntCodecHeaderTest){
  Document* doc = new Document(0,0,"");
  EXPECT_EQ(0U,doc->text().size());
  delete doc;
}
