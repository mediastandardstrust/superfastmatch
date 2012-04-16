#ifndef _SFMTESTS_H                       // duplication check
#define _SFMTESTS_H

#include <stdexcept>
#include <sys/time.h>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdint.h>
#include <cctype>
#include <kcprotodb.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <instrumentation.h>
#include <document.h>
#include <command.h>
#include <association.h>
#include <postline.h>
#include <posting.h>
#include <queue.h>
#include <query.h>
#include <search.h>
#include <worker.h>
#include <api.h>
#include <mock_registry.h>

using namespace testing;
using namespace superfastmatch;
using namespace kyotocabinet;
using namespace std;
using namespace std::tr1;

class BaseTest :public ::testing::Test{
protected:
  PolyDB* associationDB_;
  PolyDB* documentDB_;
  PolyDB* metaDB_;
  PolyDB* orderedMetaDB_;
  PolyDB* queueDB_;
  PolyDB* payloadDB_;
  PolyDB* miscDB_;
  DocumentManager* documentManager_;
  AssociationManager* associationManager_;
  QueueManager* queueManager_;
  Posting* postings_;
  MockRegistry registry_;
  Logger* logger_;
  Api* api_;
  TemplateCache* templates_;
  InstrumentGroupPtr instrumentGroup_;
  
  virtual void SetUp();
  virtual void TearDown();

};

// Testing apparatus
class TestDocument{
private:
  string text_;
  string form_text_;
  unordered_map<size_t,size_t> uniques_;
public:
  TestDocument(const char* filename);
  string& getText();
  string& getFormText();
  size_t getUniques(const size_t window_size);
};

typedef shared_ptr<TestDocument> TestDocumentPtr;

#endif
