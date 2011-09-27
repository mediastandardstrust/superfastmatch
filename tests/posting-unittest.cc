#include <tests.h>
#include <document.h>
#include <posting.h>

class PostingTest : public ::testing::Test{
protected:
  PolyDB* documentDB_;
  PolyDB* metaDB_;
  Posting* postings_;
  DocumentManager* documentManager_;
  Logger* logger_;
  MockRegistry registry_;
  
  virtual void SetUp(){
    documentDB_ = new PolyDB();
    metaDB_ = new PolyDB();
    documentDB_->open("%");
    metaDB_->open("%");
    EXPECT_CALL(registry_,getWindowSize())
      .WillRepeatedly(Return(10));
    EXPECT_CALL(registry_,getDocumentDB())
      .WillRepeatedly(Return(documentDB_));
    EXPECT_CALL(registry_,getMetaDB())
      .WillRepeatedly(Return(metaDB_));
    EXPECT_CALL(registry_,getWhiteSpaceThreshold())
      .WillRepeatedly(Return(4));
    EXPECT_CALL(registry_,getWhiteSpaceHash(false))
      .WillRepeatedly(Return(0));
    EXPECT_CALL(registry_,getWhiteSpaceHash(true))
      .WillRepeatedly(Return(0));
    EXPECT_CALL(registry_,getMaxPostingThreshold())
      .WillRepeatedly(Return(200));
    EXPECT_CALL(registry_,getMaxDistance())
      .WillRepeatedly(Return(5));
    EXPECT_CALL(registry_,getSlotCount())
      .WillRepeatedly(Return(4));
    EXPECT_CALL(registry_,getHashWidth())
      .WillRepeatedly(Return(24));
    EXPECT_CALL(registry_,getHashMask())
      .WillRepeatedly(Return((1L<<24)-1));
    EXPECT_CALL(registry_,getMaxHashCount())
      .WillRepeatedly(Return(1<<24));
    EXPECT_CALL(registry_,getMaxLineLength())
      .WillRepeatedly(Return(1024));

    // These may be dependent on the above!!
    documentManager_ = new DocumentManager(&registry_);
    postings_=new Posting(&registry_);
    logger_= new Logger();
    EXPECT_CALL(registry_,getPostings())
      .WillRepeatedly(Return(postings_));
    EXPECT_CALL(registry_,getDocumentManager())
      .WillRepeatedly(Return(documentManager_));
    EXPECT_CALL(registry_,getLogger())
      .WillRepeatedly(Return(logger_));
  }
  
  virtual void TearDown(){
    metaDB_->close();
    documentDB_->close();
    delete metaDB_;
    delete documentDB_;
    delete documentManager_;
    delete postings_;
    delete logger_;
  }
  
};

TEST_F(PostingTest,SearchTest){
  DocumentPtr doc1=registry_.getDocumentManager()->createPermanentDocument(1,1,"text=This+is+a+test+with+the+same+sentence");
  DocumentPtr doc2=registry_.getDocumentManager()->createPermanentDocument(1,2,"text=Another+test+with+the+same+sentence");
  registry_.getPostings()->addDocument(doc1);
  registry_.getPostings()->addDocument(doc2);
  registry_.getPostings()->wait(0);
  DocumentPtr searchDoc=registry_.getDocumentManager()->createTemporaryDocument("text=the+same+sentence");
  EXPECT_EQ(0U,searchDoc->docid());
  EXPECT_EQ(0U,searchDoc->doctype());
  search_t results;
  inverted_search_t pruned_results;
  registry_.getPostings()->searchIndex(searchDoc,results,pruned_results);
  EXPECT_EQ(0U,searchDoc->docid());
  EXPECT_EQ(0U,searchDoc->doctype());
  EXPECT_EQ(2U,results.size());
}
