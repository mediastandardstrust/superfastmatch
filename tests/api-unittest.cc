#include <tests.h>

struct TestResponse{
  map<string,string> headers;
  string body;
};

typedef shared_ptr<TestResponse> TestResponsePtr;

TestResponsePtr TestAPI(Api* api,const HTTPClient::Method method,const string& path, const string& query,const string& body,const int expected){
  map<string,string> emptyMap;
  string emptyString;
  map<string,string> misc;
  misc["query"]=query;
  TestResponsePtr response(new TestResponse());
  EXPECT_EQ(expected,api->Invoke(path,method,emptyMap,body,response->headers,response->body,misc)) << "Wrong response code found for: " << path ;
  return response;
}

TEST_F(BaseTest,DocumentAutoIdApiTest){
  EXPECT_THAT(TestAPI(api_,HTTPClient::MPOST,"/document/4/","","text=testing+123",202)->body,HasSubstr("\"docid\"     : 1"));
  EXPECT_THAT(TestAPI(api_,HTTPClient::MPOST,"/document/4/","","text=testing+123",202)->body,HasSubstr("\"docid\"     : 2"));
  TestAPI(api_,HTTPClient::MPOST,"/document/5/","","",400);
  TestAPI(api_,HTTPClient::MPOST,"/document/5/","","text=",400);
  EXPECT_THAT(TestAPI(api_,HTTPClient::MPOST,"/document/5/","","text=testing+123",202)->body,HasSubstr("\"docid\"     : 3"));
  EXPECT_THAT(TestAPI(api_,HTTPClient::MPOST,"/document/5/","","text=testing+123",202)->body,HasSubstr("\"docid\"     : 4"));
  registry_.getQueueManager()->processQueue();
}

TEST_F(BaseTest,DocumentApiTest){
  TestAPI(api_,HTTPClient::MGET,"/document","","",200); 
  TestAPI(api_,HTTPClient::MGET,"/document/","","",200);
  TestAPI(api_,HTTPClient::MGET,"/document/5","","",200);
  TestAPI(api_,HTTPClient::MGET,"/document/5/","","",200);
  TestAPI(api_,HTTPClient::MGET,"/document/5/1","","",404);
  TestAPI(api_,HTTPClient::MGET,"/document/5/1/","","",404);
  TestAPI(api_,HTTPClient::MPOST,"/document/","","",-1);
  TestAPI(api_,HTTPClient::MPOST,"/document/5/1","","",400);
  TestAPI(api_,HTTPClient::MPOST,"/document/5/1/","","",400);
  TestAPI(api_,HTTPClient::MPOST,"/document/5/1/","","text=",400);
  TestAPI(api_,HTTPClient::MPUT,"/document/5/1/","","text=",400);
  TestAPI(api_,HTTPClient::MPOST,"/document/5/1","","text=testing+123",202);
  TestAPI(api_,HTTPClient::MPOST,"/document/5/1/","","text=testing+123",202);
  TestAPI(api_,HTTPClient::MPUT,"/document/5/1","","text=testing+123",202);
  TestAPI(api_,HTTPClient::MPUT,"/document/5/1/","","text=testing+123",202);
  registry_.getQueueManager()->processQueue();
  TestAPI(api_,HTTPClient::MGET,"/document/5/1","","",200);
  TestAPI(api_,HTTPClient::MGET,"/document/5/1/","","",200);
  TestAPI(api_,HTTPClient::MGET,"/document/5/1/","limit=10","",200);
  TestAPI(api_,HTTPClient::MGET,"/document/5/1/","order_by=-title&limit=10000","",200);
  TestAPI(api_,HTTPClient::MPOST,"/search","","",400);
  TestAPI(api_,HTTPClient::MPOST,"/search/","","",400);
  TestAPI(api_,HTTPClient::MPOST,"/search","","text=testing+123",200);
  TestAPI(api_,HTTPClient::MPOST,"/search/","","text=testing+123",200);
  TestAPI(api_,HTTPClient::MDELETE,"/document/5/1","","",202);
  TestAPI(api_,HTTPClient::MDELETE,"/document/5/1/","","",202);
  registry_.getQueueManager()->processQueue();
  TestAPI(api_,HTTPClient::MGET,"/document/5/1","","",404);
  TestAPI(api_,HTTPClient::MGET,"/document/5/1/","","",404);
  TestAPI(api_,HTTPClient::MGET,"/document/0/0/","","",400);
  TestAPI(api_,HTTPClient::MGET,"/document/0/1/","","",400);
  TestAPI(api_,HTTPClient::MGET,"/document/1/0/","","",400); 
  TestAPI(api_,HTTPClient::MPUT,"/document/0/1","","text=testing+123",400);
  TestAPI(api_,HTTPClient::MPUT,"/document/1/0/","","text=testing+123",400);
  TestAPI(api_,HTTPClient::MPUT,"/document/0/0/","","text=testing+123",400); 
  TestAPI(api_,HTTPClient::MPOST,"/document/0/1","","text=testing+123",400);
  TestAPI(api_,HTTPClient::MPOST,"/document/1/0/","","text=testing+123",400);
  TestAPI(api_,HTTPClient::MPOST,"/document/0/0/","","text=testing+123",400); 
}

TEST_F(BaseTest,DocumentPutApiTest){
  TestAPI(api_,HTTPClient::MPOST,"/document/1/1","","text=This+is+a+long+sentence+where+the+phrase+Always+Look+On+The+Bright+Side+Of+Life",202);
  TestAPI(api_,HTTPClient::MPUT,"/document/4/4/","","text=Always+Look+On+The+Bright+Side+Of+Life+and+this+is+a+long+sentence",202);
  registry_.getQueueManager()->processQueue();
  EXPECT_EQ(2,registry_.getDocumentDB()->count());
  EXPECT_EQ(2,registry_.getAssociationDB()->count());
  TestAPI(api_,HTTPClient::MDELETE,"/document/1/1","","",202);
  TestAPI(api_,HTTPClient::MDELETE,"/document/4/4/","","",202);
  registry_.getQueueManager()->processQueue();
  EXPECT_EQ(0,registry_.getDocumentDB()->count());
  EXPECT_EQ(0,registry_.getAssociationDB()->count());
}

TEST_F(BaseTest,SearchApiTest){
  TestAPI(api_,HTTPClient::MPOST,"/document/1/1","","text=This+is+a+long+sentence+where+the+phrase+Always+Look+On+The+Bright+Side+Of+Life",202);
  TestAPI(api_,HTTPClient::MPOST,"/document/4/4/","","text=Always+Look+On+The+Bright+Side+Of+Life+and+this+is+a+long+sentence",202);
  registry_.getQueueManager()->processQueue();
  EXPECT_EQ(2,registry_.getDocumentDB()->count());
  string result=TestAPI(api_,HTTPClient::MPOST,"/search/","","text=Always+Look+On+The+Bright+Side+Of+Life",200)->body;
  EXPECT_THAT(result,HasSubstr("\"docid\": 1"));
  EXPECT_THAT(result,HasSubstr("\"doctype\": 1"));
  EXPECT_THAT(result,HasSubstr("\"docid\": 4"));
  EXPECT_THAT(result,HasSubstr("\"doctype\": 4"));
  result=TestAPI(api_,HTTPClient::MPOST,"/search/1/","","text=Always+Look+On+The+Bright+Side+Of+Life",200)->body;
  EXPECT_THAT(result,HasSubstr("\"docid\": 1"));
  EXPECT_THAT(result,HasSubstr("\"doctype\": 1"));
  EXPECT_THAT(result,Not(HasSubstr("\"docid\": 4")));
  EXPECT_THAT(result,Not(HasSubstr("\"doctype\": 4")));
  TestAPI(api_,HTTPClient::MPOST,"/search/1-43-5/","","text=Always+Look+On+The+Bright+Side+Of+Life",400);
  TestAPI(api_,HTTPClient::MPOST,"/search/1/","","teasxt=Always+Look+On+The+Bright+Side+Of+Life",400);
}

TEST_F(BaseTest,AssociateAllDocumentsApiTest){
  TestAPI(api_,HTTPClient::MPOST,"/document/1/1","","text=This+is+a+long+sentence+where+the+phrase+Always+Look+On+The+Bright+Side+Of+Life",202);
  TestAPI(api_,HTTPClient::MPOST,"/document/4/4/","","text=Always+Look+On+The+Bright+Side+Of+Life+and+this+is+a+long+sentence",202);
  registry_.getQueueManager()->processQueue();
  EXPECT_EQ(2,registry_.getDocumentDB()->count());
  TestAPI(api_,HTTPClient::MPOST,"/associations/","","",202);
  registry_.getQueueManager()->processQueue();
  EXPECT_EQ(2,registry_.getAssociationDB()->count());
}

TEST_F(BaseTest,AssociateDocumentRangeApiTest){
  TestAPI(api_,HTTPClient::MPOST,"/document/1/1","","text=This+is+a+long+sentence+where+the+phrase+Always+Look+On+The+Bright+Side+Of+Life",202);
  TestAPI(api_,HTTPClient::MPOST,"/document/4/4/","","text=Always+Look+On+The+Bright+Side+Of+Life+and+this+is+a+long+sentence",202);
  TestAPI(api_,HTTPClient::MPOST,"/document/5/1/","","text=Always+Look+On+The+Bright+Side+Of+Life",202);
  registry_.getQueueManager()->processQueue();
  EXPECT_EQ(3,registry_.getDocumentDB()->count());
  TestAPI(api_,HTTPClient::MPOST,"/associations/1/4/","","",202);
  registry_.getQueueManager()->processQueue();
  EXPECT_EQ(2,registry_.getAssociationDB()->count());
  TestAPI(api_,HTTPClient::MPOST,"/associations/1-4/","","",202);
  registry_.getQueueManager()->processQueue();
  EXPECT_EQ(6,registry_.getAssociationDB()->count());
  TestAPI(api_,HTTPClient::MPOST,"/associations/5/1:4/","","",202);
  registry_.getQueueManager()->processQueue();
  EXPECT_EQ(6,registry_.getAssociationDB()->count());
}

TEST_F(BaseTest,QueueApiTest){
  TestAPI(api_,HTTPClient::MGET,"/queue","","",200);
  TestAPI(api_,HTTPClient::MGET,"/queue/","","",200);
  TestAPI(api_,HTTPClient::MGET,"/QUEUE/","","",200);
  TestAPI(api_,HTTPClient::MGET,"/queue/","start=0","",200);
  TestAPI(api_,HTTPClient::MGET,"/queue/","start=0&limit=0","",200);
  TestAPI(api_,HTTPClient::MGET,"/queue/","start=0&limit=10","",200);
  TestAPI(api_,HTTPClient::MGET,"/queue/","limit=10","",200);
}

TEST_F(BaseTest,IndexApiTest){
  TestAPI(api_,HTTPClient::MGET,"/index","","",200);
  TestAPI(api_,HTTPClient::MGET,"/index/","","",200);
  TestAPI(api_,HTTPClient::MGET,"/INDEX/","","",200);
}

// TEST_F(BaseTest,PerformanceApiTest){
//   TestAPI(api_,HTTPClient::MGET,"/performance","","",200);
//   TestAPI(api_,HTTPClient::MGET,"/performance/","","",200);
//   TestAPI(api_,HTTPClient::MGET,"/PERFORMANCE/","","",200);
// }

TEST_F(BaseTest,StatusApiTest){
  TestAPI(api_,HTTPClient::MGET,"/status","","",200);
  TestAPI(api_,HTTPClient::MGET,"/status/","","",200);
  TestAPI(api_,HTTPClient::MGET,"/STATUS/","","",200);
}

TEST_F(BaseTest,AssociateNonExistentDocumentApiTest){
  TestAPI(api_,HTTPClient::MPOST,"/association/1/1/","","",202);
  registry_.getQueueManager()->processQueue();
}

TEST_F(BaseTest,RandomAddDeleteApiSlowTest){
  for (size_t i=1;i<=10000;i++){
    bool operation=rand()%2!=1;
    uint32_t doctype=rand()%10+2147483648;
    uint32_t docid=rand()%(8192)+2147483648;
    if (operation){
      TestAPI(api_,HTTPClient::MPOST,"/document/"+toString(doctype)+"/"+toString(docid),"","text=This+is+a+long+sentence+where+the+phrase+Always+Look+On+The+Bright+Side+Of+Life",202);    
    }else{
      TestAPI(api_,HTTPClient::MDELETE,"/document/"+toString(doctype)+"/"+toString(docid),"","",202);    
    }
  }
  registry_.getQueueManager()->processQueue();  
}

