#include <tests.h>

typedef BaseTest DocumentQueryTest;

void testDocTypeRange(const string& range, const uint32_t count,const bool valid){
  DocTypeRange r;
  EXPECT_EQ(valid,r.parse(range)) << "Range = " << ::testing::PrintToString(range);;
  EXPECT_EQ(count,r.getDocTypes().size()) << "Range = " << ::testing::PrintToString(range);
}

void testQuery(Registry* registry,const string& command,const uint32_t source_count, const uint32_t target_count, const bool valid){
  DocumentQuery query(registry,command);
  EXPECT_EQ(source_count,query.getSourceDocPairs().size()) << "Command = " << ::testing::PrintToString(command);
  EXPECT_EQ(target_count,query.getTargetDocPairs().size()) << "Command = " << ::testing::PrintToString(command);
  EXPECT_EQ(valid,query.isValid()) << "Command = " << ::testing::PrintToString(command);
}

TEST_F(DocumentQueryTest,DocRangeParsingTest){
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

TEST_F(DocumentQueryTest,ParsingTest){
  DocumentPtr doc1=registry_.getDocumentManager()->createPermanentDocument(1,1,"text=This+is+a+test&title=Also+a+test&filename=test.txt");
  DocumentPtr doc2=registry_.getDocumentManager()->createPermanentDocument(1,2,"text=Another+test&title=Also+a+test&filename=test2.txt");
  DocumentPtr doc3=registry_.getDocumentManager()->createPermanentDocument(1,3,"text=Final+test&title=Also+a+test&filename=test3.txt");
  DocumentPtr doc4=registry_.getDocumentManager()->createPermanentDocument(2,1,"text=This+is+a+test&title=Also+a+test&filename=test.txt");
  DocumentPtr doc5=registry_.getDocumentManager()->createPermanentDocument(2,2,"text=Another+test&title=Also+a+test&filename=test2.txt");
  DocumentPtr doc6=registry_.getDocumentManager()->createPermanentDocument(2,3,"text=Final+test&title=Also+a+test&filename=test3.txt");  
  // testQuery(&registry_,"",0,0,true);
  // testQuery(&registry_,"1/1/",1,1,true);
  // testQuery(&registry_,"1-2/1-2/",2,2,true);
}