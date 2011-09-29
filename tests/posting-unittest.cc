#include <tests.h>

typedef BaseTest PostingTest;

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
