#include <tests.h>

typedef BaseTest DocumentQueryTest;

void testDocTypeRange(const string& range, const uint32_t count,const bool valid){
  DocTypeRange r;
  EXPECT_EQ(valid,r.parse(range)) << "Range = " << ::testing::PrintToString(range);;
  EXPECT_EQ(count,r.getDocTypes().size()) << "Range = " << ::testing::PrintToString(range);
}

void testQuery(Registry* registry,const string& command,const uint32_t source_count, const uint32_t target_count, const bool valid,const string& cursor, const string order, const bool desc, const uint32_t limit){
  DocumentQuery query(registry,command);
  EXPECT_EQ(source_count,query.getSourceDocPairs().size()) << "Command = " << ::testing::PrintToString(command);
  EXPECT_EQ(target_count,query.getTargetDocPairs().size()) << "Command = " << ::testing::PrintToString(command);
  EXPECT_EQ(valid,query.isValid()) << "Command = " << ::testing::PrintToString(command);
  EXPECT_STREQ(cursor.c_str(),query.getCursor().c_str()) << "Command = " << ::testing::PrintToString(command);
  EXPECT_STREQ(order.c_str(),query.getOrder().c_str()) << "Command = " << ::testing::PrintToString(command);
  EXPECT_EQ(desc,query.isDescending()) << "Command = " << ::testing::PrintToString(command);
  EXPECT_EQ(limit,query.getLimit()) << "Command = " << ::testing::PrintToString(command);
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

// TEST_F(DocumentQueryTest,EndToEndTest){
//   uint32_t limit=registry_.getPageSize();
//   DocumentPtr doc1=registry_.getDocumentManager()->createPermanentDocument(1,1,"text=This+is+a+test&title=Also+a+test&filename=test.txt");
//   DocumentPtr doc2=registry_.getDocumentManager()->createPermanentDocument(1,2,"text=Another+test&title=Also+a+test&filename=test2.txt");
//   DocumentPtr doc3=registry_.getDocumentManager()->createPermanentDocument(1,3,"text=Final+test&title=Final+test&filename=test3.txt");
//   DocumentPtr doc4=registry_.getDocumentManager()->createPermanentDocument(2,1,"text=This+is+a+test&title=Also+a+test&filename=test.txt");
//   DocumentPtr doc5=registry_.getDocumentManager()->createPermanentDocument(2,2,"text=Another+test&title=Also+a+test&filename=test2.txt");
//   DocumentPtr doc6=registry_.getDocumentManager()->createPermanentDocument(2,3,"text=Final+test&title=Final+test&filename=test3.txt");  
//   testQuery(&registry_,"http://test.com/document/",6,6,true,"","",false,limit);
//   testQuery(&registry_,"http://test.com/document/1/2/",3,3,true,"","",false,limit);
//   testQuery(&registry_,"http://test.com/document/1-2/1-2/",6,6,true,"","",false,limit);
//   testQuery(&registry_,"http://test.com/document/1-2/1-2/?cursor=Also+a+test",6,6,true,"Also a test","",false,limit);
//   testQuery(&registry_,"http://test.com/document/1-2/1-2/?order_by=title",6,6,true,"","title",false,limit);
//   testQuery(&registry_,"http://test.com/document/1-2/1-2/?order_by=-title",6,6,true,"","title",true,limit);
//   testQuery(&registry_,"http://test.com/document/1-2/1-2/?limit=2",2,2,true,"","",false,2);
//   testQuery(&registry_,"http://test.com/document/1-2/1-2/?cursor=Final+test&order_by=title&limit=50",2,2,true,"Final test","title",false,50);
//   testQuery(&registry_,"http://test.com/document/1-2/1-2/?cursor=Final+test&order_by=-title&limit=50",6,6,true,"Final test","title",true,50);
//   testQuery(&registry_,"http://test.com/document/1-2/1-2/?cursor=Final+test&order_by=-title&limit=50",6,6,true,"Final test","title",true,50);
//   testQuery(&registry_,"http://test.com/document/1/",3,6,true,"","",false,limit);
// }
// 
// TEST_F(DocumentQueryTest,PagingTest){
//   for (size_t i=1;i<=4;i++){
//     for (size_t j=1;j<=300;j++){
//       stringstream s;
//       s << "text=This+is+a+test&title=Test" << setfill('0') << setw(10) << j;
//       registry_.getDocumentManager()->createPermanentDocument(i,j,s.str());
//     } 
//   }
//   uint32_t limit=registry_.getPageSize();
//   testQuery(&registry_,"http://test.com/document/",limit,limit,true,"","",false,limit);
//   testQuery(&registry_,"http://test.com/document/?limit=1000",1000,1000,true,"","",false,1000);
//   testQuery(&registry_,"http://test.com/document/1/?limit=1000",300,1000,true,"","",false,1000);
//   testQuery(&registry_,"http://test.com/document/1/1/?limit=1000",300,300,true,"","",false,1000);
//   testQuery(&registry_,"http://test.com/document/?limit=100",100,100,true,"","",false,100);
//   
//   DocumentQuery query(&registry_,"http://test.com/document/?limit=100");
//   EXPECT_STREQ("/document/?limit=100",query.getFirst().c_str());
//   // 
//   DocumentQuery query2(&registry_,"http://test.com/document/1/?order_by=title&limit=100");
//   EXPECT_STREQ("/document/1/?limit=100&order_by=title",query2.getFirst().c_str());
//   EXPECT_STREQ("/document/1/?limit=100&order_by=title&cursor=Test0000000001%00%00%00%01%00%00%00%01",query2.getPrevious().c_str());
//   EXPECT_STREQ("/document/1/?limit=100&order_by=title&cursor=Test0000000101%00%00%00%01%00%00%00e",query2.getNext().c_str());
//   EXPECT_STREQ("/document/1/?limit=100&order_by=title&cursor=Test0000000201%00%00%00%01%00%00%00%c9",query2.getLast().c_str());
//   
//   DocumentQuery query3(&registry_,"http://test.com/document/1/?order_by=-title&limit=100");
//   EXPECT_STREQ("/document/1/?limit=100&order_by=-title",query3.getFirst().c_str());
//   EXPECT_STREQ("/document/1/?limit=100&order_by=-title",query3.getPrevious().c_str());
//   EXPECT_STREQ("/document/1/?limit=100&order_by=-title&cursor=Test0000000201%00%00%00%01%00%00%00%c9",query3.getNext().c_str());
//   EXPECT_STREQ("/document/1/?limit=100&order_by=-title&cursor=Test0000000100%00%00%00%01%00%00%00d",query3.getLast().c_str());
// 
//   DocumentQuery query4(&registry_,"http://test.com/document/2/?order_by=-title&limit=100");
//   EXPECT_STREQ("/document/2/?limit=100&order_by=-title",query4.getFirst().c_str());
//   EXPECT_STREQ("/document/2/?limit=100&order_by=-title",query4.getPrevious().c_str());
//   EXPECT_STREQ("/document/2/?limit=100&order_by=-title&cursor=Test0000000201%00%00%00%02%00%00%00%c9",query4.getNext().c_str());
//   EXPECT_STREQ("/document/2/?limit=100&order_by=-title&cursor=Test0000000100%00%00%00%02%00%00%00d",query4.getLast().c_str());
//   
//   DocumentQuery query5(&registry_,"http://test.com/document/2/?order_by=-title&limit=100&cursor=Test0000000201%00%00%00%02%00%00%00%c9");
//   EXPECT_STREQ("/document/2/?limit=100&order_by=-title",query5.getFirst().c_str());
//   EXPECT_STREQ("/document/2/?limit=100&order_by=-title&cursor=Test0000000100%00%00%00%02%00%00%00d",query5.getPrevious().c_str());
//   EXPECT_STREQ("/document/2/?limit=100&order_by=-title&cursor=Test0000000300%00%00%00%02%00%00%00%c9",query5.getNext().c_str());
//   EXPECT_STREQ("/document/2/?limit=100&order_by=-title&cursor=Test0000000200%00%00%00%02%00%00%00d",query5.getLast().c_str());
// }