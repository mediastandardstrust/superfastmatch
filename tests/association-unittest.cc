#include <tests.h>

typedef BaseTest AssociationTest;

TEST_F(AssociationTest,ShortTest){
  EXPECT_CALL(registry_,getWindowSize())
    .WillRepeatedly(Return(4));
  DocumentPtr doc1 = registry_.getDocumentManager()->createPermanentDocument(1,1,"text=12345This1");
  DocumentPtr doc2 = registry_.getDocumentManager()->createPermanentDocument(1,2,"text=12this2");
  Association* association = new Association(&registry_,doc1,doc2);
  EXPECT_EQ(1U,association->getResultCount());
  EXPECT_EQ(4U,association->getResult(0).length);
  EXPECT_EQ(4U,association->getResult(0).uc_length);
  EXPECT_EQ(2U,association->getResult(0).right);
  EXPECT_EQ(2U,association->getResult(0).uc_right);
  EXPECT_EQ(5U,association->getResult(0).left);
  EXPECT_EQ(5U,association->getResult(0).uc_left);
  EXPECT_STREQ("This",association->getFromResultText(0).c_str());
  EXPECT_STREQ("this",association->getToResultText(0).c_str());
}

TEST_F(AssociationTest,UTF8Test){
  EXPECT_CALL(registry_,getWindowSize())
    .WillRepeatedly(Return(4));
  DocumentPtr doc1 = registry_.getDocumentManager()->createPermanentDocument(1,1,"text=\xf0\x90\x8d\x86\xe6\x97\xa5\xd1\x88+This+is+a+Test");
  DocumentPtr doc2 = registry_.getDocumentManager()->createPermanentDocument(1,2,"text=Unicode\xf0\x90\x8d\x86\xe6\x97\xa5\xd1\x88");
  Association* association = new Association(&registry_,doc1,doc2);
  EXPECT_EQ(1U,association->getResultCount());
  EXPECT_EQ(9U,association->getResult(0).length);
  EXPECT_EQ(3U,association->getResult(0).uc_length);
  EXPECT_EQ(0U,association->getResult(0).left);
  EXPECT_EQ(0U,association->getResult(0).uc_left);
  EXPECT_EQ(7U,association->getResult(0).right);
  EXPECT_EQ(7U,association->getResult(0).uc_right);
  EXPECT_STREQ("\xf0\x90\x8d\x86\xe6\x97\xa5\xd1\x88",association->getFromResultText(0).c_str());
  EXPECT_STREQ("\xf0\x90\x8d\x86\xe6\x97\xa5\xd1\x88",association->getToResultText(0).c_str());
}

TEST_F(AssociationTest,ConstructorTest){
  DocumentPtr doc1 = registry_.getDocumentManager()->createPermanentDocument(1,1,"text=This+is+a+long+sentence+where+the+phrase+Always+Look+On+The+Bright+Side+Of+Life&title=Doc1");
  DocumentPtr doc2 = registry_.getDocumentManager()->createPermanentDocument(1,2,"text=Always+Look+On+The+Bright+Side+Of+Life+and+this+is+a+long+sentence&title=Doc2");
  EXPECT_STREQ("This is a long sentence where the phrase Always Look On The Bright Side Of Life",doc1->getText().c_str());
  EXPECT_STREQ("Always Look On The Bright Side Of Life and this is a long sentence",doc2->getText().c_str());
  Association* association = new Association(&registry_,doc1,doc2);
  EXPECT_EQ(2U,association->getResultCount());
  EXPECT_EQ(38U,association->getResult(0).length);
  EXPECT_STREQ("Always Look On The Bright Side Of Life",association->getToResultText(0).c_str());
  EXPECT_STREQ(association->getFromResultText(0).c_str(),association->getToResultText(0).c_str());
  EXPECT_EQ(23U,association->getResult(1).length);
  EXPECT_STREQ("This is a long sentence",association->getFromResultText(1).c_str());
  EXPECT_STRCASEEQ(association->getToResultText(1).c_str(),association->getFromResultText(1).c_str());
  EXPECT_EQ(61U,association->getTotalLength());
  EXPECT_EQ(2U,registry_.getDocumentDB()->count());
  EXPECT_EQ(0U,registry_.getAssociationDB()->count());
  EXPECT_EQ(true,association->save());
  EXPECT_EQ(2U,registry_.getDocumentDB()->count());
  EXPECT_EQ(2U,registry_.getAssociationDB()->count());
  Association* association2 = new Association(&registry_,doc1,doc2);
  EXPECT_EQ(2U,association2->getResultCount());  
  Association* association3 = new Association(&registry_,doc2,doc1);
  EXPECT_EQ(2U,association3->getResultCount());
  EXPECT_STRCASEEQ(association2->getFromResultText(1).c_str(),association3->getFromResultText(1).c_str());
  EXPECT_EQ(2U,registry_.getAssociationDB()->count());
}

TEST_F(AssociationTest, WhitespaceTest){
  DocumentPtr doc1 = registry_.getDocumentManager()->createTemporaryDocument("text=++++++++++++++++++++whitespace+test+with+a+long+sentence+*+*+*+*+*+*+*+*+*+*+*+*+");
  DocumentPtr doc2 = registry_.getDocumentManager()->createTemporaryDocument("text=++++++++++++++++++++whitespace+test+with+a+long+sentence+*+*+*+*+*+*+*+*+*+*+*+*+");
  Association* association = new Association(&registry_,doc1,doc2);
  EXPECT_STREQ("whitespace test with a long sentence",association->getToResultText(0).c_str());
}

TEST_F(AssociationTest, EndingTest){
  DocumentPtr doc1 = registry_.getDocumentManager()->createTemporaryDocument("text=This+is+Captain+Franklin");
  DocumentPtr doc2 = registry_.getDocumentManager()->createTemporaryDocument("text=This+is+Captain+Francis");
  Association* association = new Association(&registry_,doc1,doc2);
  EXPECT_STREQ("This is Captain Fran",association->getToResultText(0).c_str());
}

TEST_F(AssociationTest, ManagerPermanentTest){
  DocumentPtr doc1 = registry_.getDocumentManager()->createPermanentDocument(1,1,"text=This+is+a+long+sentence+where+the+phrase+Always+Look+On+The+Bright+Side+Of+Life&title=Doc1");
  DocumentPtr doc2 = registry_.getDocumentManager()->createPermanentDocument(1,2,"text=Always+Look+On+The+Bright+Side+Of+Life+and+this+is+a+long+sentence&title=Doc2");
  registry_.getPostings()->addDocument(doc1);
  registry_.getPostings()->addDocument(doc2);
  registry_.getPostings()->finishTasks();
  EXPECT_NE(0U,registry_.getPostings()->getHashCount());
  DocumentQueryPtr query(new DocumentQuery(&registry_,"",""));
  SearchPtr search=Search::createPermanentSearch(&registry_,doc1->doctype(),doc1->docid(),query);
  EXPECT_EQ(1U,search->associations.size());
  EXPECT_EQ(2U,registry_.getAssociationDB()->count());
  DocumentPtr doc3 = registry_.getDocumentManager()->getDocument(1,1);
  EXPECT_NE(0U,doc3->getText().size());
  vector<AssociationPtr> savedAssociations = registry_.getAssociationManager()->getAssociations(doc3,DocumentManager::NONE);
  EXPECT_EQ(1U,savedAssociations.size());
  EXPECT_TRUE(registry_.getAssociationManager()->removeAssociations(doc3));
  EXPECT_EQ(0U,registry_.getAssociationDB()->count());
}

TEST_F(AssociationTest, ManagerTemporaryTest){
  DocumentPtr doc1 = registry_.getDocumentManager()->createPermanentDocument(1,1,"text=This+is+a+long+sentence+where+the+phrase+Always+Look+On+The+Bright+Side+Of+Life&title=Doc1");
  registry_.getPostings()->addDocument(doc1);
  registry_.getPostings()->finishTasks();
  EXPECT_NE(0U,registry_.getPostings()->getHashCount());
  DocumentQueryPtr target(new DocumentQuery(&registry_,"",""));
  SearchPtr search=Search::createTemporarySearch(&registry_,"text=Always+Look+On+The+Bright+Side+Of+Life+and+this+is+a+long+sentence&title=Doc2",target);
  EXPECT_EQ(1U,search->associations.size());
  EXPECT_EQ(0U,registry_.getAssociationDB()->count());
}

TEST_F(AssociationTest,DanteTest){
  EXPECT_CALL(registry_,getWindowSize())
    .WillRepeatedly(Return(30));
  EXPECT_CALL(registry_,getWhiteSpaceThreshold())
    .WillRepeatedly(Return(15));
  vector<uint32_t> hashes1;
  vector<uint32_t> hashes2;
  UpperCaseRabinKarp("the\nfirst",3,3,hashes1);
  UpperCaseRabinKarp("the first",3,3,hashes2);
  EXPECT_THAT(hashes1,ContainerEq(hashes2));
  DocumentPtr doc1 = registry_.getDocumentManager()->createPermanentDocument(1,1,"text=I%20need%20not%20dilate%20here%20on%20the%20characteristics%20of%20the%0Afirst%20epoch%20of%20Italian%20Poetry%3B%20since%20the%20extent&title=Doc1");
  DocumentPtr doc2 = registry_.getDocumentManager()->createPermanentDocument(1,2,"text=I%20need%20not%20dilate%20here%20on%20the%20characteristics%20of%0Athe%20first%20epoch%20of%20Italian%20Poetry%3B%20since%20the%20extent&title=Doc2");
  EXPECT_EQ(100U,doc2->getText().length());
  EXPECT_EQ(doc1->getText().length(),doc2->getText().length());
  registry_.getPostings()->addDocument(doc1);
  registry_.getPostings()->addDocument(doc2);
  registry_.getPostings()->finishTasks();
  DocumentQueryPtr query(new DocumentQuery(&registry_,"",""));
  SearchPtr search=Search::createPermanentSearch(&registry_,doc1->doctype(),doc1->docid(),query);
  EXPECT_EQ(1U,search->associations.size());
  EXPECT_EQ(100U,search->associations[0]->getResult(0).length);
  EXPECT_EQ(2U,registry_.getAssociationDB()->count());
}
