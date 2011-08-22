#include <document.h>
#include <mock_registry.h>
#include <kcprotodb.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;
using namespace superfastmatch;
using namespace kyotocabinet;

TEST(PostingTest,SearchTest){
  PolyDB* documentDB = new PolyDB();
  PolyDB* metaDB = new PolyDB();
  documentDB->open();
  metaDB->open();
  MockRegistry registry;
  EXPECT_CALL(registry,getWindowSize())
    .WillRepeatedly(Return(10));
  EXPECT_CALL(registry,getDocumentDB())
    .WillRepeatedly(Return(documentDB));
  EXPECT_CALL(registry,getMetaDB())
    .WillRepeatedly(Return(metaDB));
  EXPECT_CALL(registry,getWhiteSpaceThreshold())
    .WillRepeatedly(Return(4));
  EXPECT_CALL(registry,getWhiteSpaceHash(false))
    .WillRepeatedly(Return(0));
  EXPECT_CALL(registry,getWhiteSpaceHash(true))
    .WillRepeatedly(Return(0));
  EXPECT_CALL(registry,getMaxPostingThreshold())
    .WillRepeatedly(Return(200));
  EXPECT_CALL(registry,getMaxDistance())
    .WillRepeatedly(Return(5));
  EXPECT_CALL(registry,getSlotCount())
    .WillRepeatedly(Return(4));
  EXPECT_CALL(registry,getHashWidth())
    .WillRepeatedly(Return(24));
  EXPECT_CALL(registry,getHashMask())
    .WillRepeatedly(Return((1L<<24)-1));
  EXPECT_CALL(registry,getMaxHashCount())
    .WillRepeatedly(Return(1<<24));
  EXPECT_CALL(registry,getMaxLineLength())
    .WillRepeatedly(Return(1024));  
  EXPECT_CALL(registry,getPostings())
    .WillRepeatedly(Return(new Posting(&registry)));
  EXPECT_CALL(registry,getLogger())
    .WillRepeatedly(Return(new Logger()));
  Document* doc1 = new Document(1,1,"text=This+is+a+test+with+the+same+sentence",&registry);
  Document* doc2 = new Document(1,2,"text=Another+test+with+the+same+sentence",&registry);
  doc1->hashes();
  doc2->hashes();
  doc1->save();
  doc2->save();
  registry.getPostings()->addDocument(doc1);
  registry.getPostings()->addDocument(doc2);
  registry.getPostings()->wait();
  Document* searchDoc = new Document(0,0,"text=the+same+sentence",&registry);
  EXPECT_EQ(0U,searchDoc->docid());
  EXPECT_EQ(0U,searchDoc->doctype());
  search_t results;
  inverted_search_t pruned_results;
  registry.getPostings()->searchIndex(searchDoc,results,pruned_results);
  EXPECT_EQ(0U,searchDoc->docid());
  EXPECT_EQ(0U,searchDoc->doctype());
  EXPECT_EQ(2U,results.size());
  delete searchDoc;
  metaDB->close();
  documentDB->close();
  delete metaDB;
  delete documentDB;
}

int main(int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}