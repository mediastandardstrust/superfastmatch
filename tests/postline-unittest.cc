#include <tests.h>
#include <vector>

TEST(PostLineTest,AllocationTest){
  size_t block;
  EXPECT_FALSE(needsAllocation(5,6,8,block));
  EXPECT_EQ(8U,block);
  EXPECT_FALSE(needsAllocation(6,7,8,block));
  EXPECT_EQ(8U,block);
  EXPECT_FALSE(needsAllocation(7,8,8,block));
  EXPECT_EQ(8U,block);
  EXPECT_FALSE(needsAllocation(8,8,8,block));
  EXPECT_EQ(8U,block);
  EXPECT_TRUE(needsAllocation(8,9,8,block));
  EXPECT_EQ(16U,block);
  EXPECT_TRUE(needsAllocation(0,9,8,block));
  EXPECT_EQ(16U,block);
  EXPECT_TRUE(needsAllocation(1,9,8,block));
  EXPECT_EQ(16U,block);
  EXPECT_TRUE(needsAllocation(19,1,8,block));
  EXPECT_EQ(8U,block);
  EXPECT_TRUE(needsAllocation(19,0,8,block));
  EXPECT_EQ(0U,block);
  EXPECT_TRUE(needsAllocation(8,0,8,block));
  EXPECT_EQ(0U,block);
  EXPECT_TRUE(needsAllocation(7,0,8,block));
  EXPECT_EQ(0U,block);
}

TEST(PostLineTest,VarIntCodecHeaderTest){
  std::vector<PostLineHeader> header;
  PostLineHeader item;
  for (size_t i=1;i<=100;i++){
    item.doc_type=i;
    item.length=i;
    header.push_back(item);
  }
  unsigned char* out = new unsigned char[1024];
  VarIntCodec codec;
  EXPECT_EQ(201U,codec.encodeHeader(header,out));
  EXPECT_EQ(201U,codec.decodeHeader(out,header));
  EXPECT_EQ(100U,header.size());
  EXPECT_EQ(1U,header[0].doc_type);
  EXPECT_EQ(1U,header[0].length);
  EXPECT_EQ(100U,header[99].doc_type);
  EXPECT_EQ(100U,header[99].length);
  delete[] out;
}  

TEST(PostLineTest,VarIntCodecSectionTest){
  const uint32_t values[] = {12,8,56,2,12998,3434};
  vector<uint32_t> section(values,values+sizeof(values)/sizeof(uint32_t));
  const size_t length=section.size();
  unsigned char* out = new unsigned char[8];
  VarIntCodec codec;
  EXPECT_EQ(8U,codec.encodeSection(section,out));
  EXPECT_EQ(length,section.size());
  EXPECT_EQ(8U,codec.decodeSection(out,8,section,true));
  EXPECT_EQ(length,section.size());
  EXPECT_EQ(values[0],section[0]);
  EXPECT_EQ(values[5],section[5]);
  delete[] out;
}

TEST(PostLineTest,GroupVarIntCodecSectionTest){
  const uint32_t values[] = {12,8,56,2,12998,3434};
  vector<uint32_t> section(values,values+sizeof(values)/sizeof(uint32_t));
  const size_t length=section.size();
  unsigned char* out = new unsigned char[16];
  GroupVarIntCodec codec;
  EXPECT_EQ(12U,codec.encodeSection(section,out));
  EXPECT_EQ(length,section.size());
  EXPECT_EQ(12U,codec.decodeSection(out,12,section,true));
  EXPECT_EQ(length,section.size());
  EXPECT_EQ(values[0],section[0]);
  EXPECT_EQ(values[5],section[5]);
  EXPECT_EQ(12U,codec.decodeSection(out,12,section,false));
  EXPECT_EQ(length,section.size());
  EXPECT_EQ(values[0],section[0]);
  EXPECT_EQ(uint32_t(accumulate(values,values+6,0)),section[5]);
  delete[] out;
}

TEST(PostLineTest,PostLineTest){
  unsigned char* in = new unsigned char[256];
  unsigned char* out = new unsigned char[256];
  vector<uint32_t>* deltas;
  vector<uint32_t>* docids;
  memset(in,0,256);
  memset(out,0,256);
  PostLine* line = new PostLine(512);
  EXPECT_EQ(0U,line->getLength());
  line->load(in);
  line->addDocument(2,1);
  EXPECT_NE(0U,line->getLength(2));
  EXPECT_NE(0U,line->getLength());
  ASSERT_TRUE(line->commit(out));
  deltas=line->getDeltas(2);
  EXPECT_THAT(*deltas,ElementsAre(1));
  docids=line->getDocIds(2);
  EXPECT_THAT(*deltas,ElementsAre(1));
  // Add new document
  line->addDocument(2,7);
  ASSERT_TRUE(line->commit(out));
  deltas=line->getDeltas(2);
  EXPECT_THAT(*deltas,ElementsAre(1,6));
  docids=line->getDocIds(2);
  EXPECT_THAT(*docids,ElementsAre(1,7));
  docids=line->getDocIds(1);
  EXPECT_THAT(docids->size(),0);
  // Add new document
  line->addDocument(2,3);
  ASSERT_TRUE(line->commit(out));
  deltas=line->getDeltas(2);
  EXPECT_THAT(*deltas,ElementsAre(1,2,4));
  docids=line->getDocIds(2);
  EXPECT_THAT(*docids,ElementsAre(1,3,7));
  // Add new document with new doc type
  line->addDocument(1,3);
  ASSERT_TRUE(line->commit(out));
  docids=line->getDocIds(1);
  EXPECT_THAT(*docids,ElementsAre(3));
  docids=line->getDocIds(2);
  EXPECT_THAT(*docids,ElementsAre(1,3,7));
  // Add new document before
  line->addDocument(1,1);
  ASSERT_TRUE(line->commit(out));
  docids=line->getDocIds(1);
  EXPECT_THAT(*docids,ElementsAre(1,3));
  docids=line->getDocIds(2);
  EXPECT_THAT(*docids,ElementsAre(1,3,7));
  // Add new document at end
  line->addDocument(1,9);
  ASSERT_TRUE(line->commit(out));
  docids=line->getDocIds(1);
  EXPECT_THAT(*docids,ElementsAre(1,3,9));
  docids=line->getDocIds(2);
  EXPECT_THAT(*docids,ElementsAre(1,3,7));
  // Add new document with new doc type
  line->addDocument(5,9);
  ASSERT_TRUE(line->commit(out));
  docids=line->getDocIds(1);
  EXPECT_THAT(*docids,ElementsAre(1,3,9));
  docids=line->getDocIds(2);
  EXPECT_THAT(*docids,ElementsAre(1,3,7));
  docids=line->getDocIds(5);
  EXPECT_THAT(*docids,ElementsAre(9));
  // Commit with no change
  ASSERT_FALSE(line->commit(out));
  docids=line->getDocIds(1);
  EXPECT_THAT(*docids,ElementsAre(1,3,9));
  docids=line->getDocIds(2);
  EXPECT_THAT(*docids,ElementsAre(1,3,7));
  docids=line->getDocIds(5);
  EXPECT_THAT(*docids,ElementsAre(9));
  // Delete a document
  line->deleteDocument(2,3);
  line->commit(out);
  docids=line->getDocIds(1);
  EXPECT_THAT(*docids,ElementsAre(1,3,9));
  docids=line->getDocIds(2);
  EXPECT_THAT(*docids,ElementsAre(1,7));
  docids=line->getDocIds(5);
  EXPECT_THAT(*docids,ElementsAre(9));
  // Delete a document
  line->deleteDocument(5,9);
  line->commit(out);
  docids=line->getDocIds(1);
  EXPECT_THAT(*docids,ElementsAre(1,3,9));
  docids=line->getDocIds(2);
  EXPECT_THAT(*docids,ElementsAre(1,7));
  docids=line->getDocIds(5);
  EXPECT_EQ(0U,docids->size());
  delete[] out;
  delete[] in;
  delete line;
}

TEST(PostLineTest,NonExistentDocumentPostLineTest){
  unsigned char* first = new unsigned char[8];
  memset(first,0,8);
  PostLine* line = new PostLine(512);
  line->load(first);
  EXPECT_EQ(1U,line->getLength());
  // Delete non existent document
  ASSERT_FALSE(line->deleteDocument(2,56));
  EXPECT_EQ(1U,line->getLength());
  delete[] first;
  delete line;
}

TEST(PostLineTest,RealisticPostLineTest){
  unsigned char* first = new unsigned char[32];
  unsigned char* second = new unsigned char[32];
  memset(first,0,32);
  memset(second,0,32);
  vector<uint32_t>* docids;
  PostLine* line = new PostLine(512);
  line->load(first);
  // This is the single 0 marking the end of the header!
  EXPECT_EQ(1U,line->getLength());
  line->addDocument(2,1);
  EXPECT_EQ(4U,line->getLength());
  line->commit(first);
  EXPECT_EQ(4U,line->getLength());
  line->addDocument(3,97);
  EXPECT_EQ(7U,line->getLength());
  line->commit(first);
  EXPECT_EQ(7U,line->getLength());
  line->addDocument(4,65);
  EXPECT_EQ(10U,line->getLength());
  line->commit(second);
  memset(first,0,32);
  EXPECT_EQ(10U,line->getLength());
  line->addDocument(4,64);
  line->commit(second);
  docids=line->getDocIds(2);
  EXPECT_THAT(*docids,ElementsAre(1));
  docids=line->getDocIds(3);
  EXPECT_THAT(*docids,ElementsAre(97));
  docids=line->getDocIds(4);
  EXPECT_THAT(*docids,ElementsAre(64,65));
  line->deleteDocument(2,1);
  line->commit(second);
  EXPECT_EQ(8U,line->getLength());
  docids=line->getDocIds(2);
  EXPECT_EQ(0U,docids->size());
  line->deleteDocument(3,97);
  EXPECT_EQ(5U,line->getLength());
  line->commit(second);
  line->deleteDocument(4,64);
  docids=line->getDocIds(4);
  EXPECT_EQ(1U,docids->size());
  line->commit(second);
  line->deleteDocument(4,65);
  line->commit(first);
  EXPECT_EQ(1U,line->getLength());
  delete[] first;
  delete[] second;
  delete line;
}

TEST(PostLineTest,BigPostLineTest){
  unsigned char* forwards = new unsigned char[4096];
  unsigned char* backwards = new unsigned char[4096];
  vector<uint32_t>* f_deltas;
  vector<uint32_t>* b_deltas;
  memset(forwards,0,4096);
  memset(backwards,0,4096);
  PostLine* line = new PostLine(4096*5);
  line->load(forwards);
  for (size_t doc_type=1;doc_type<12;doc_type++){
    for (size_t doc_id=1;doc_id<=256;doc_id++){
      line->addDocument(doc_type,doc_id);
      line->commit(forwards);
    }
  }
  line->load(backwards);
  for (size_t doc_type=12;doc_type>0;doc_type--){
    for (size_t doc_id=256;doc_id>0;doc_id--){
      line->addDocument(doc_type,doc_id);
      line->commit(backwards);
    }
  }
  for (size_t doc_type=1;doc_type<12;doc_type++){
    line->load(forwards);
    f_deltas=line->getDeltas(doc_type);
    line->load(backwards);
    b_deltas=line->getDeltas(doc_type);
    EXPECT_EQ(f_deltas->size(),b_deltas->size());
    EXPECT_THAT(*f_deltas,ContainerEq(*b_deltas));
  }
  delete[] forwards;
  delete[] backwards;
  delete line;
}

TEST(PostLineTest,RandomLineSlowTest){
  unordered_map<uint32_t,set<uint32_t> > control;
  const size_t SIZE=4096;
  unsigned char* data = new unsigned char[SIZE+8];
  memset(data,0,SIZE+8);
  PostLine* line = new PostLine(SIZE);
  line->load(data);
  for (size_t i=0;i<(1UL<<18);i++){
    bool operation=rand()%3!=1;
    uint32_t doctype=rand()%10+1;
    uint32_t docid=rand()%(4096)+1;
    if (operation){
        if (line->getLength()<(SIZE-15)){
          line->addDocument(doctype,docid);
          control[doctype].insert(docid);
        }
      }else{
        line->deleteDocument(doctype,docid);
        control[doctype].erase(docid);
        if (control[doctype].size()==0){
          control.erase(doctype);
        }
      }
    EXPECT_EQ(0U,*(uint64_t*)&data[SIZE]);
    line->commit(data);
    EXPECT_EQ(0U,*(uint64_t*)&data[SIZE]);
    line->load(data);
  }
  vector<PostLineHeader>* headers=line->load(data);
  for (vector<PostLineHeader>::const_iterator it=headers->begin(),ite=headers->end();it!=ite;++it){
    EXPECT_EQ(it->length,control[it->doc_type].size());
    vector<uint32_t>* docids=line->getDocIds(it->doc_type);
    EXPECT_THAT(control[it->doc_type],ContainerEq(set<uint32_t>(docids->begin(),docids->end())));
  }
  delete[] data;
  delete line;
} 




