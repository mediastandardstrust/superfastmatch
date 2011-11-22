#include <tests.h>

typedef BaseTest PostingTest;

TEST_F(PostingTest,SearchTest){
  DocumentPtr doc1=registry_.getDocumentManager()->createPermanentDocument(1,1,"text=This+is+a+test+with+the+same+sentence");
  DocumentPtr doc2=registry_.getDocumentManager()->createPermanentDocument(1,2,"text=Another+test+with+the+same+sentence");
  registry_.getPostings()->addDocument(doc1);
  registry_.getPostings()->addDocument(doc2);
  registry_.getPostings()->finishTasks();
  SearchPtr search=Search::createTemporarySearch(&registry_,"text=the+same+sentence");
  EXPECT_EQ(0U,search->doc->doctype());
  EXPECT_EQ(0U,search->doc->docid());
  EXPECT_EQ(2U,search->results.size());
}
