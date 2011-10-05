#ifndef _SFMTESTS_H                       // duplication check
#define _SFMTESTS_H

#include <stdexcept>
#include <kcprotodb.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <document.h>
#include <command.h>
#include <association.h>
#include <postline.h>
#include <posting.h>
#include <queue.h>
#include <query.h>
#include <mock_registry.h>

using namespace testing;
using namespace superfastmatch;
using namespace kyotocabinet;

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
  
  virtual void SetUp();
  virtual void TearDown();

};

#endif
