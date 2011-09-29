#include <tests.h>

void testDocTypeRange(const string& range, const uint32_t count,const bool valid){
  DocTypeRange r;
  EXPECT_EQ(valid,r.parse(range)) << "Range = " << ::testing::PrintToString(range);;
  EXPECT_EQ(count,r.getDocTypes().size()) << "Range = " << ::testing::PrintToString(range);
}

TEST(DocumentQueryTest,DocRangeParsingTest){
  testDocTypeRange("",0,true);
  testDocTypeRange("1",1,true);
  testDocTypeRange("1;",0,false);
  testDocTypeRange("1-10",10,true);
  testDocTypeRange("1-10;21-40",30,true);
  testDocTypeRange("0",0,false);
  testDocTypeRange("1-0",0,false);
  testDocTypeRange("6-5",1,true);
  testDocTypeRange("6-5;1-10",10,true);
  testDocTypeRange("6-5;1-10;",0,false);
  testDocTypeRange("6-5-34;1-10",0,false);
}

void testQuery(const string& command,const uint32_t source_count, const uint32_t target_count, const bool valid){
  MockRegistry registry;
  DocumentQuery query(&registry,command);
  EXPECT_EQ(source_count,query.getSourceDocPairs().size()) << "Command = " << ::testing::PrintToString(command);
  EXPECT_EQ(target_count,query.getTargetDocPairs().size()) << "Command = " << ::testing::PrintToString(command);
  EXPECT_EQ(valid,query.isValid()) << "Command = " << ::testing::PrintToString(command);
}

typedef BaseTest DocumentQueryTest;

TEST(DocumentQueryTest,ParsingTest){
  // testQuery("",0,0,true);
  // testQuery("1/1/",1,1,true);
  // testQuery("1-2/1-2/",2,2,true);
}