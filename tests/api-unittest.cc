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

TEST_F(BaseTest,DocumentApiTest){
  TestAPI(api_,HTTPClient::MGET,"/document","","",200); 
  TestAPI(api_,HTTPClient::MGET,"/document/","","",200);
  TestAPI(api_,HTTPClient::MGET,"/document/5","","",200);
  TestAPI(api_,HTTPClient::MGET,"/document/5/","","",200);
  TestAPI(api_,HTTPClient::MGET,"/document/5/1","","",404);
  TestAPI(api_,HTTPClient::MGET,"/document/5/1/","","",404);
  TestAPI(api_,HTTPClient::MPOST,"/document/","","",-1);
  TestAPI(api_,HTTPClient::MPOST,"/document/5/1","","",500);
  TestAPI(api_,HTTPClient::MPOST,"/document/5/1/","","",500);
  TestAPI(api_,HTTPClient::MPOST,"/document/5/1","","text=testing+123",202);
  TestAPI(api_,HTTPClient::MPOST,"/document/5/1/","","text=testing+123",202);
  TestAPI(api_,HTTPClient::MPUT,"/document/5/1","","text=testing+123",202);
  TestAPI(api_,HTTPClient::MPUT,"/document/5/1/","","text=testing+123",202);
  registry_.getQueueManager()->processQueue();
  TestAPI(api_,HTTPClient::MGET,"/document/5/1","","",200);
  TestAPI(api_,HTTPClient::MGET,"/document/5/1/","","",200);
  TestAPI(api_,HTTPClient::MPOST,"/search","","",500);
  TestAPI(api_,HTTPClient::MPOST,"/search/","","",500);
  TestAPI(api_,HTTPClient::MPOST,"/search","","text=testing+123",200);
  TestAPI(api_,HTTPClient::MPOST,"/search/","","text=testing+123",200);
  TestAPI(api_,HTTPClient::MDELETE,"/document/5/1","","",202);
  TestAPI(api_,HTTPClient::MDELETE,"/document/5/1/","","",202);
  registry_.getQueueManager()->processQueue();
  TestAPI(api_,HTTPClient::MGET,"/document/5/1","","",404);
  TestAPI(api_,HTTPClient::MGET,"/document/5/1/","","",404);
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
  // TestAPI(api_,HTTPClient::MPOST,"/associations/1-4/","","",202);
  // registry_.getQueueManager()->processQueue();
  // EXPECT_EQ(6,registry_.getAssociationDB()->count());
  // TestAPI(api_,HTTPClient::MPOST,"/associations/5/1:4/","","",202);
  // registry_.getQueueManager()->processQueue();
  // EXPECT_EQ(6,registry_.getAssociationDB()->count());
}

// TEST_F(BaseTest,AssociateDocumentsApiTest){
//   TestAPI(api_,HTTPClient::MPOST,"/document/1/1","","text=This+is+a+long+sentence+where+the+phrase+Always+Look+On+The+Bright+Side+Of+Life",202);
//   TestAPI(api_,HTTPClient::MPOST,"/document/4/4/","","text=Always+Look+On+The+Bright+Side+Of+Life+and+this+is+a+long+sentence",202);
//   registry_.getQueueManager()->processQueue();
//   EXPECT_EQ(2,registry_.getDocumentDB()->count());
//   TestAPI(api_,HTTPClient::MPOST,"/association/1/1/","","",202);
//   registry_.getQueueManager()->processQueue();
//   EXPECT_EQ(2,registry_.getAssociationDB()->count());
//   TestAPI(api_,HTTPClient::MPOST,"/association/4/4/","","",202);
//   registry_.getQueueManager()->processQueue();
//   EXPECT_EQ(2,registry_.getAssociationDB()->count());
// }

TEST_F(BaseTest,QueueApiTest){
  TestAPI(api_,HTTPClient::MGET,"/queue","","",200);
  TestAPI(api_,HTTPClient::MGET,"/queue/","","",200);
  TestAPI(api_,HTTPClient::MGET,"/QUEUE/","","",200);
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

TEST_F(BaseTest,HistogramApiTest){
  TestAPI(api_,HTTPClient::MGET,"/histogram","","",200);
  TestAPI(api_,HTTPClient::MGET,"/histogram/","","",200);
}




