#include <tests.h>

typedef BaseTest DocumentTest;
typedef DocumentTest DocumentDeathTest;

TEST_F(DocumentTest,isNumericTest){
  EXPECT_TRUE(isNumeric("123"));
  EXPECT_TRUE(isNumeric("-123"));
  EXPECT_TRUE(isNumeric("123.0"));
  EXPECT_FALSE(isNumeric("1.a"));
  EXPECT_FALSE(isNumeric(".0"));
  EXPECT_FALSE(isNumeric("1."));
  EXPECT_FALSE(isNumeric("..0"));
  EXPECT_FALSE(isNumeric("..."));
  EXPECT_FALSE(isNumeric("."));
}

TEST_F(DocumentTest,ConstructorTest){
  DocumentPtr doc1=registry_.getDocumentManager()->createPermanentDocument(1,1,"text=This+is+a+test&title=Also+a+test&filename=test.txt");
  DocumentPtr doc2=registry_.getDocumentManager()->createPermanentDocument(1,2,"text=Another+test&title=Also+a+test&filename=test2.txt");
  EXPECT_EQ(14U,doc1->getText().size());
  EXPECT_STREQ("This is a test",doc1->getText().c_str());
  EXPECT_STREQ("THIS IS A TEST",doc1->getCleanText().c_str());
  EXPECT_STREQ("Another test",doc2->getText().c_str());
  EXPECT_STREQ("ANOTHER TEST",doc2->getCleanText().c_str());
  EXPECT_EQ(11U,doc1->getMeta("title").size());
  EXPECT_STREQ("Also a test",doc1->getMeta("title").c_str());
  EXPECT_EQ(8U,doc1->getMeta("filename").size());
  EXPECT_STREQ("test.txt",doc1->getMeta("filename").c_str());
  DocumentPtr doc3 =registry_.getDocumentManager()->getDocument(1,1);
  EXPECT_STREQ("test.txt",doc3->getMeta("filename").c_str());
  EXPECT_EQ(doc3->getText().size()-registry_.getWindowSize()+1,doc3->getHashes().size());
  EXPECT_STREQ("Also a test",doc3->getMeta("title").c_str());
  EXPECT_STREQ("This is a test",doc3->getText().c_str());
}

TEST_F(DocumentTest, CleanTextTest){
  DocumentPtr doc=registry_.getDocumentManager()->createTemporaryDocument("text=This+is+a+test+with+*+*++***^$$#@@<>lots+of+whitespace");
  EXPECT_EQ(doc->getText().size(),doc->getCleanText().size());
  EXPECT_EQ("THIS IS A TEST WITH                 LOTS OF WHITESPACE",doc->getCleanText());
}

TEST_F(DocumentTest, MetaTest){
  string value;
  vector<string> keys;
  DocumentPtr doc1=registry_.getDocumentManager()->createPermanentDocument(1,1,"text=some+short+text+with_metadata&title=Title&filename=test.txt");
  EXPECT_STREQ("Title",doc1->getMeta("title").c_str());
  EXPECT_STREQ("test.txt",doc1->getMeta("filename").c_str());
  EXPECT_TRUE(doc1->setMeta("count","4"));
  EXPECT_STREQ("1",doc1->getMeta("docid").c_str());
  EXPECT_STREQ("1",doc1->getMeta("doctype").c_str());
  EXPECT_STREQ("4",doc1->getMeta("count").c_str());
  EXPECT_STRNE("",doc1->getMeta("characters").c_str());
  EXPECT_STREQ("",doc1->getMeta("non-existent").c_str());
  EXPECT_TRUE(doc1->getMetaKeys(keys));
  EXPECT_EQ(6U,keys.size());
  EXPECT_EQ(6U,registry_.getMetaDB()->count());
  EXPECT_EQ(6U,registry_.getOrderedMetaDB()->count());
  DocumentPtr doc2=registry_.getDocumentManager()->getDocument(1,1,DocumentManager::META);
  EXPECT_STREQ("4",doc2->getMeta("count").c_str());
  EXPECT_STREQ("1",doc1->getMeta("docid").c_str());
  EXPECT_STREQ("1",doc1->getMeta("doctype").c_str());
  EXPECT_STREQ("Title",doc2->getMeta("title").c_str());
  EXPECT_STREQ("test.txt",doc2->getMeta("filename").c_str());
  EXPECT_TRUE(doc1->getMetaKeys(keys));
  EXPECT_EQ(6U,keys.size());
  EXPECT_EQ(6U,registry_.getMetaDB()->count());
  EXPECT_EQ(6U,registry_.getOrderedMetaDB()->count());
  DocumentPtr doc3=registry_.getDocumentManager()->createPermanentDocument(1,2,"text=some+short+text+with_metadata&title=Title&filename=test.txt");
  EXPECT_EQ(11U,registry_.getMetaDB()->count());
  EXPECT_EQ(11U,registry_.getOrderedMetaDB()->count());
  EXPECT_TRUE(registry_.getDocumentManager()->removePermanentDocument(doc1));
  EXPECT_EQ(5U,registry_.getMetaDB()->count());
  EXPECT_EQ(5U,registry_.getOrderedMetaDB()->count());
  EXPECT_TRUE(registry_.getDocumentManager()->removePermanentDocument(doc3));
  EXPECT_EQ(0U,registry_.getMetaDB()->count());
  EXPECT_EQ(0U,registry_.getOrderedMetaDB()->count());
}

TEST_F(DocumentTest,PaddingTest){
  string paddedNumber("0000000010");
  string number("10");
  string alphaNumeric("a10");
  EXPECT_EQ(paddedNumber,padIfNumber(number));
  EXPECT_EQ(alphaNumeric,padIfNumber(alphaNumeric));
}

// TEST_F(DocumentTest, OrderedMetaTest){
//   DocumentPtr doc1=registry_.getDocumentManager()->createPermanentDocument(1,1,"text=definitely+the+longest&title=C&filename=test.txt");
//   DocumentPtr doc2=registry_.getDocumentManager()->createPermanentDocument(1,2,"text=shortest&title=B&filename=test2.txt");
//   DocumentPtr doc3=registry_.getDocumentManager()->createPermanentDocument(1,3,"text=medium+length&title=A&filename=test3.txt");
//   EXPECT_EQ(27U,registry_.getMetaDB()->count());
//   DocumentCursor cursor(&registry_);
//   EXPECT_EQ(1U,cursor.getNext()->docid());
//   EXPECT_EQ(2U,cursor.getNext()->docid());
//   EXPECT_EQ(3U,cursor.getNext()->docid());
//   EXPECT_FALSE(cursor.getNext());
//   DocumentCursor cursor2(&registry_,"title",Document::FORWARD);
//   EXPECT_EQ(3U,cursor2.getNext()->docid());
//   EXPECT_EQ(2U,cursor2.getNext()->docid());
//   EXPECT_EQ(1U,cursor2.getNext()->docid());
//   EXPECT_FALSE(cursor2.getNext());
//   DocumentCursor cursor3(&registry_,"title",Document::REVERSE);
//   EXPECT_EQ(1U,cursor3.getNext()->docid());
//   EXPECT_EQ(2U,cursor3.getNext()->docid());
//   EXPECT_EQ(3U,cursor3.getNext()->docid());
//   EXPECT_FALSE(cursor3.getNext());
//   DocumentCursor cursor4(&registry_,"characters",Document::FORWARD);
//   EXPECT_STREQ("shortest",cursor4.getNext(DocumentManager::TEXT)->getText().c_str());
//   EXPECT_STREQ("medium length",cursor4.getNext(DocumentManager::TEXT)->getText().c_str());
//   EXPECT_STREQ("definitely the longest",cursor4.getNext(DocumentManager::TEXT)->getText().c_str());
//   EXPECT_FALSE(cursor4.getNext());
// }

TEST_F(DocumentTest,ManagerTest){
  DocumentPtr tempDoc=registry_.getDocumentManager()->createTemporaryDocument("text=some+short+text+with_metadata&title=Title");
  DocumentPtr permDoc=registry_.getDocumentManager()->createPermanentDocument(1,1,"text=some+short+text+with_metadata&title=Title");
  DocumentPtr savedDoc=registry_.getDocumentManager()->getDocument(1,1);
  DocumentPtr savedDocFromKey=registry_.getDocumentManager()->getDocument(permDoc->getKey());
  EXPECT_STREQ("Title",tempDoc->getMeta("title").c_str());
  EXPECT_STREQ("Title",permDoc->getMeta("title").c_str());
  EXPECT_STREQ("Title",savedDoc->getMeta("title").c_str());
  EXPECT_STREQ("Title",savedDocFromKey->getMeta("title").c_str());
  EXPECT_EQ(0U,tempDoc->doctype());
  EXPECT_EQ(0U,tempDoc->docid());
  EXPECT_EQ(1U,permDoc->doctype());
  EXPECT_EQ(1U,permDoc->docid());
  EXPECT_EQ(1U,savedDoc->doctype());
  EXPECT_EQ(1U,savedDoc->docid());
  EXPECT_EQ(1U,savedDocFromKey->doctype());
  EXPECT_EQ(1U,savedDocFromKey->docid());
  EXPECT_NE(0U,tempDoc->getHashes().size());
  EXPECT_NE(0U,permDoc->getHashes().size());
  EXPECT_NE(0U,savedDoc->getHashes().size());
  EXPECT_NE(0U,savedDocFromKey->getHashes().size());
}

TEST_F(DocumentDeathTest,RemovalTest){
  DocumentPtr permDoc=registry_.getDocumentManager()->createPermanentDocument(1,1,"text=some+short+text+with_metadata&title=Title");
  EXPECT_EQ(1U,permDoc->doctype());
  EXPECT_EQ(1U,permDoc->docid());
  EXPECT_STREQ("Title",permDoc->getMeta("title").c_str());
  EXPECT_TRUE(registry_.getDocumentManager()->removePermanentDocument(permDoc));
  EXPECT_FALSE(registry_.getDocumentManager()->getDocument(1,1));
}

TEST_F(DocumentTest,DuplicatePermanentTest){
  DocumentPtr permDoc=registry_.getDocumentManager()->createPermanentDocument(1,1,"text=some+short+text+with_metadata&title=Title");
  EXPECT_EQ(1U,permDoc->doctype());
  EXPECT_EQ(1U,permDoc->docid());
  EXPECT_STREQ("Title",permDoc->getMeta("title").c_str());
  EXPECT_FALSE(registry_.getDocumentManager()->createPermanentDocument(1,1,"text=some+short+text+with_metadata&title=Title"));
}

TEST_F(DocumentTest,InitTest){
  DocumentPtr tempDoc=registry_.getDocumentManager()->createTemporaryDocument("text=Some+short+text+with+metadata&title=Title",NULL);
  EXPECT_STREQ("Some short text with metadata",tempDoc->getText().c_str());
  EXPECT_STREQ("Title",tempDoc->getMeta("title").c_str());
  ASSERT_NO_THROW(tempDoc->getCleanText());
  ASSERT_THROW(tempDoc->getHashes(),std::runtime_error);
  ASSERT_THROW(tempDoc->getBloom(),std::runtime_error);
  tempDoc=registry_.getDocumentManager()->createTemporaryDocument("text=Some+short+text+with+metadata&title=Title",DocumentManager::TEXT);
  EXPECT_STREQ("Some short text with metadata",tempDoc->getText().c_str());
  EXPECT_STREQ("Title",tempDoc->getMeta("title").c_str());
  EXPECT_STREQ("SOME SHORT TEXT WITH METADATA",tempDoc->getCleanText().c_str());
  ASSERT_THROW(tempDoc->getHashes(),std::runtime_error);
  ASSERT_THROW(tempDoc->getBloom(),std::runtime_error);
  tempDoc=registry_.getDocumentManager()->createTemporaryDocument("text=Some+short+text+with+metadata&title=Title",DocumentManager::TEXT|DocumentManager::HASHES);
  EXPECT_STREQ("Some short text with metadata",tempDoc->getText().c_str());
  EXPECT_STREQ("Title",tempDoc->getMeta("title").c_str());
  EXPECT_STREQ("SOME SHORT TEXT WITH METADATA",tempDoc->getCleanText().c_str());
  EXPECT_GT(tempDoc->getHashes().size(),0U);
  ASSERT_THROW(tempDoc->getBloom(),std::runtime_error);
  tempDoc=registry_.getDocumentManager()->createTemporaryDocument("text=Some+short+text+with+metadata&title=Title",DocumentManager::TEXT|DocumentManager::HASHES|DocumentManager::BLOOM);
  EXPECT_STREQ("Some short text with metadata",tempDoc->getText().c_str());
  EXPECT_STREQ("Title",tempDoc->getMeta("title").c_str());
  EXPECT_STREQ("SOME SHORT TEXT WITH METADATA",tempDoc->getCleanText().c_str());
  EXPECT_GT(tempDoc->getHashes().size(),0U);
  EXPECT_GT(tempDoc->getBloom().size(),0U);
  ASSERT_THROW(registry_.getDocumentManager()->createTemporaryDocument("text=Some+short+text+with+metadata&title=Title",DocumentManager::BLOOM),std::runtime_error);
}

TEST_F(DocumentTest,DocumentListTest){
  DocumentPtr doc1=registry_.getDocumentManager()->createPermanentDocument(1,1,"text=This+is+a+test&title=...&filename=test.txt");
  DocumentPtr doc2=registry_.getDocumentManager()->createPermanentDocument(1,2,"text=Another+test&title=Also+a+test&filename=test2.txt");
  DocumentPtr doc3=registry_.getDocumentManager()->createPermanentDocument(2,1,"text=This+is+a+test&title=Also+a+test&filename=test.txt");
  DocumentPtr doc4=registry_.getDocumentManager()->createPermanentDocument(2,2,"text=Another+test&title=Also+a+test&filename=test2.txt");
  set<string> metadata;
  TemplateDictionary dict("test");
  TemplateDictionary* sourceDict=dict.AddSectionDictionary("SOURCE");
  registry_.getDocumentManager()->getDocument(1,1,DocumentManager::TEXT|DocumentManager::META)->fillJSONDictionary(sourceDict,metadata);
  string output;
  EXPECT_GT(metadata.size(),0U);
  EXPECT_TRUE(ctemplate::ExpandTemplate("templates/JSON/search.tpl",STRIP_WHITESPACE, &dict, &output));
  EXPECT_THAT(output,HasSubstr("\"...\""));
}
