#include <tests.h>

typedef BaseTest DocumentQueryTest;

void testDocTypeRange(const string& range, const uint32_t count,const bool valid){
  DocTypeRange r;
  EXPECT_EQ(valid,r.parse(range)) << "Range = " << ::testing::PrintToString(range);;
  EXPECT_EQ(count,r.getDocTypes().size()) << "Range = " << ::testing::PrintToString(range);
}

DocumentQueryPtr buildQuery(Registry* registry,const string& source, const string& target,const string& querystring){
  ApiParams params(HTTPClient::MGET,"");
  vector<string> queries,parts;
  strsplit(querystring,"&",&queries);
  for (vector<string>::const_iterator it=queries.begin();it!=queries.end();++it){
    strsplit(*it,"=",&parts);
    if (parts.size()==2){
      size_t ksiz,vsiz;
      char* kbuf = urldecode(parts[0].c_str(), &ksiz);
      char* vbuf = urldecode(parts[1].c_str(), &vsiz);
      params.query[kbuf]=vbuf;
      delete[] kbuf;
      delete[] vbuf;
    }
  } 
  return DocumentQueryPtr(new DocumentQuery(registry,source,target,params.query));
}

DocumentQueryPtr testQuery(Registry* registry,const string& source, const string& target,const string& querystring,const uint32_t source_count, const uint32_t target_count, const bool valid,const string& cursor, const string order, const bool desc, const uint32_t limit){
  DocumentQueryPtr query=buildQuery(registry,source,target,querystring);
  string command="/"+source+"/"+target+"/"+querystring;
  EXPECT_EQ(source_count,query->getSourceDocPairs().size()) << "Command = " << ::testing::PrintToString(command);
  EXPECT_EQ(target_count,query->getTargetDocPairs().size()) << "Command = " << ::testing::PrintToString(command);
  EXPECT_EQ(valid,query->isValid()) << "Command = " << ::testing::PrintToString(command);
  EXPECT_STREQ(cursor.c_str(),query->getCursor().c_str()) << "Command = " << ::testing::PrintToString(command);
  EXPECT_STREQ(order.c_str(),query->getOrder().c_str()) << "Command = " << ::testing::PrintToString(command);
  EXPECT_EQ(desc,query->isDescending()) << "Command = " << ::testing::PrintToString(command);
  EXPECT_EQ(limit,query->getLimit()) << "Command = " << ::testing::PrintToString(command);
  return query;
}

TEST_F(DocumentQueryTest,DocRangeParsingTest){
  testDocTypeRange("",0,true);
  testDocTypeRange("1",1,true);
  testDocTypeRange("1:",0,false);
  testDocTypeRange("1-10",10,true);
  testDocTypeRange("1-10:21-40",30,true);
  testDocTypeRange("0",0,false);
  testDocTypeRange("1-0",0,false);
  testDocTypeRange("6-5",2,true);
  testDocTypeRange("6-5:1-10",10,true);
  testDocTypeRange("6-5:1-10:",0,false);
  testDocTypeRange("6-5-34:1-10",0,false);
}

TEST_F(DocumentQueryTest,EndToEndTest){
  uint32_t limit=registry_.getPageSize();
  DocumentPtr doc1=registry_.getDocumentManager()->createPermanentDocument(1,1,"text=This+is+a+test&title=Also+a+test&filename=test.txt");
  DocumentPtr doc2=registry_.getDocumentManager()->createPermanentDocument(1,2,"text=Another+test&title=Also+a+test&filename=test2.txt");
  DocumentPtr doc3=registry_.getDocumentManager()->createPermanentDocument(1,3,"text=Final+test&title=Final+test&filename=test3.txt");
  DocumentPtr doc4=registry_.getDocumentManager()->createPermanentDocument(2,1,"text=This+is+a+test&title=Also+a+test&filename=test.txt");
  DocumentPtr doc5=registry_.getDocumentManager()->createPermanentDocument(2,2,"text=Another+test&title=Also+a+test&filename=test2.txt");
  DocumentPtr doc6=registry_.getDocumentManager()->createPermanentDocument(2,3,"text=Final+test&title=Final+test&filename=test3.txt");  
  testQuery(&registry_,"","","",6,6,true,"","doctype",false,limit);
  testQuery(&registry_,"1","","",3,6,true,"","doctype",false,limit);
  testQuery(&registry_,"1-2","1-2","",6,6,true,"","doctype",false,limit);
  testQuery(&registry_,"2-1","2-1","",6,6,true,"","doctype",false,limit);
  testQuery(&registry_,"1:2","1:2","",6,6,true,"","doctype",false,limit);
  testQuery(&registry_,"2:1","2:1","",6,6,true,"","doctype",false,limit);
  testQuery(&registry_,"2:1-5","2:1-5","",6,6,true,"","doctype",false,limit);
  testQuery(&registry_,"1-2","1-2","order_by=title",6,6,true,"","title",false,limit);
  testQuery(&registry_,"1-2","1-2","order_by=title&cursor=Also+a+test",6,6,true,"Also a test","title",false,limit);
  testQuery(&registry_,"1-2","1-2","order_by=title&cursor=Final+test",2,2,true,"Final test","title",false,limit);
  testQuery(&registry_,"1-2","1-2","order_by=-title",6,6,true,"","title",true,limit);
  testQuery(&registry_,"1-2","1-2","limit=2",2,2,true,"","doctype",false,2);
  testQuery(&registry_,"1-2","1-2","cursor=Final+test&order_by=title&limit=50",2,2,true,"Final test","title",false,50);
  testQuery(&registry_,"1-2","1-2","cursor=Final+test&order_by=-title&limit=50",6,6,true,"Final test","title",true,50);
  testQuery(&registry_,"1-2","1-2","cursor=Final+test&order_by=-title&limit=50",6,6,true,"Final test","title",true,50);
  testQuery(&registry_,"","","limit=10000&order_by=-title",6,6,true,"","title",true,10000);
  testQuery(&registry_,"1","","",3,6,true,"","doctype",false,limit);
}

TEST_F(DocumentQueryTest,PagingTest){
  for (size_t i=1;i<=4;i++){
    for (size_t j=1;j<=300;j++){
      stringstream s;
      s << "text=This+is+a+test&title=Test" << setfill('0') << setw(10) << j;
      registry_.getDocumentManager()->createPermanentDocument(i,j,s.str());
    } 
  }
  uint32_t limit=registry_.getPageSize();
  testQuery(&registry_,"","","",limit,limit,true,"","doctype",false,limit);
  testQuery(&registry_,"","","limit=1000",1000,1000,true,"","doctype",false,1000);
  testQuery(&registry_,"1","","limit=1000",300,1000,true,"","doctype",false,1000);
  testQuery(&registry_,"1","1","limit=1000",300,300,true,"","doctype",false,1000);
  testQuery(&registry_,"","","limit=100",100,100,true,"","doctype",false,100);

  testQuery(&registry_,"","","cursor=1:1:1",limit,limit,true,"1:1:1","doctype",false,limit);
  testQuery(&registry_,"","","cursor=1:1",limit,limit,true,"1:1","doctype",false,limit);
  testQuery(&registry_,"","","cursor=1",limit,limit,true,"1","doctype",false,limit);
  testQuery(&registry_,"","","cursor=1:1:2",limit,limit,true,"1:1:2","doctype",false,limit);
  
  DocumentQueryPtr query=buildQuery(&registry_,"","","limit=100");
  EXPECT_STREQ("",query->getFirst().c_str());

  DocumentQueryPtr query2=buildQuery(&registry_,"1","","order_by=title&limit=100");
  EXPECT_STREQ("",query2->getFirst().c_str());
  EXPECT_STREQ("Test0000000001:1:1",query2->getPrevious().c_str());
  EXPECT_STREQ("Test0000000101:1:101",query2->getNext().c_str());
  EXPECT_STREQ("Test0000000201:1:201",query2->getLast().c_str());
  
  DocumentQueryPtr query3=buildQuery(&registry_,"1","","order_by=-title&limit=100");
  EXPECT_STREQ("",query3->getFirst().c_str());
  EXPECT_STREQ("Test0000000300:1:300",query3->getPrevious().c_str());
  EXPECT_STREQ("Test0000000200:1:200",query3->getNext().c_str());
  EXPECT_STREQ("Test0000000100:1:100",query3->getLast().c_str());

  DocumentQueryPtr query4=buildQuery(&registry_,"2","","order_by=-title&limit=100");
  EXPECT_STREQ("",query4->getFirst().c_str());
  EXPECT_STREQ("Test0000000300:2:300",query4->getPrevious().c_str());
  EXPECT_STREQ("Test0000000200:2:200",query4->getNext().c_str());
  EXPECT_STREQ("Test0000000100:2:100",query4->getLast().c_str());
  
  DocumentQueryPtr query5=buildQuery(&registry_,"2","","order_by=-title&limit=100&cursor=Test0000000201:2:201");
  EXPECT_STREQ("",query5->getFirst().c_str());
  EXPECT_STREQ("Test0000000300:2:300",query5->getPrevious().c_str());
  EXPECT_STREQ("Test0000000101:2:101",query5->getNext().c_str());
  EXPECT_STREQ("Test0000000100:2:100",query5->getLast().c_str());
}
