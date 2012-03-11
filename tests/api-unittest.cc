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
  TestAPI(api_,HTTPClient::MGET,"document","","",200);
  TestAPI(api_,HTTPClient::MGET,"document/","","",200);
  TestAPI(api_,HTTPClient::MGET,"document/5","","",200);
  TestAPI(api_,HTTPClient::MGET,"document/5/","","",200);
  TestAPI(api_,HTTPClient::MGET,"document/5/1","","",404);
  TestAPI(api_,HTTPClient::MGET,"document/5/1/","","",404);
  TestAPI(api_,HTTPClient::MPOST,"document/","","",-1);
  TestAPI(api_,HTTPClient::MPOST,"document/5/1","","",500);
  TestAPI(api_,HTTPClient::MPOST,"document/5/1/","","",500);
  TestAPI(api_,HTTPClient::MPOST,"document/5/1","","text=testing+123",202);
  TestAPI(api_,HTTPClient::MPOST,"document/5/1/","","text=testing+123",202);
  TestAPI(api_,HTTPClient::MPUT,"document/5/1","","text=testing+123",202);
  TestAPI(api_,HTTPClient::MPUT,"document/5/1/","","text=testing+123",202);
  registry_.getQueueManager()->processQueue();
  TestAPI(api_,HTTPClient::MGET,"document/5/1","","",200);
  TestAPI(api_,HTTPClient::MGET,"document/5/1/","","",200);
  TestAPI(api_,HTTPClient::MPOST,"search","","",500);
  TestAPI(api_,HTTPClient::MPOST,"search/","","",500);
  TestAPI(api_,HTTPClient::MPOST,"search","","text=testing+123",200);
  TestAPI(api_,HTTPClient::MPOST,"search/","","text=testing+123",200);
}

TEST_F(BaseTest,StatusApiTest){
  TestAPI(api_,HTTPClient::MGET,"status","","",200);
  TestAPI(api_,HTTPClient::MGET,"status/","","",200);
  TestAPI(api_,HTTPClient::MGET,"STATUS/","","",200);
}



