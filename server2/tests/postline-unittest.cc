#include <postline.h>
#include <vector>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;
using namespace superfastmatch;

TEST(PostLineTest,VarIntCodecHeaderTest){
  std::vector<PostLineHeader> header;
  PostLineHeader item;
  for (size_t i=1;i<=100;i++){
    item.doc_type=i;
    item.length=i;
    header.push_back(item);    
  }
  unsigned char* out = new unsigned char[1024];
  PostLineCodec* codec = new VarIntCodec();
  EXPECT_EQ(201U,codec->encodeHeader(header,out));
  EXPECT_EQ(201U,codec->decodeHeader(out,header));
  EXPECT_EQ(100U,header.size());
  EXPECT_EQ(1U,header[0].doc_type);
  EXPECT_EQ(1U,header[0].length);
  EXPECT_EQ(100U,header[99].doc_type);
  EXPECT_EQ(100U,header[99].length);
  delete[] out;
}  

TEST(PostLineTest,VarIntCodecSectionTest){
  const uint32_t values[] = {12,1,56,2,12998,3434};
  std::vector<uint32_t> section(values,values+sizeof(values)/sizeof(uint32_t));
  unsigned char* out = new unsigned char[12];
  PostLineCodec* codec = new VarIntCodec();
  EXPECT_EQ(8U,codec->encodeSection(section,out));
  EXPECT_EQ(8U,codec->decodeSection(out,8,section));
  EXPECT_EQ(6U,section.size());
  EXPECT_EQ(values[0],section[0]);
  EXPECT_EQ(values[5],section[5]);
  delete[] out;
}

TEST(PostLineTest,PostLineTest){
  unsigned char* in = new unsigned char[256];
  unsigned char* out = new unsigned char[256];
  vector<uint32_t> deltas;
  vector<uint32_t> docids;
  memset(in,0,256);
  memset(out,0,256);
  size_t length;
  PostLineCodec* codec = new VarIntCodec();
  PostLine* line = new PostLine(codec,512);
  line->load(in);
  line->addDocument(2,1);
  length=line->getLength(2);
  EXPECT_EQ(1U,length);
  length=line->getLength();
  EXPECT_EQ(4U,length);
  line->commit(out);
  EXPECT_EQ(out[0],2);
  EXPECT_EQ(out[1],1);
  EXPECT_EQ(out[2],0);
  EXPECT_EQ(out[3],1);
  line->getDeltas(2,deltas);
  EXPECT_THAT(deltas,ElementsAre(1));
  line->getDocIds(2,docids);
  EXPECT_THAT(deltas,ElementsAre(1));
  line->addDocument(2,7);
  line->commit(out);
  line->getDeltas(2,deltas);
  EXPECT_THAT(deltas,ElementsAre(1,6));
  line->getDocIds(2,docids);
  EXPECT_THAT(docids,ElementsAre(1,7));
  line->getDocIds(1,docids);
  EXPECT_THAT(docids.size(),0);
  line->addDocument(2,3);
  line->commit(out);
  line->getDeltas(2,deltas);
  EXPECT_THAT(deltas,ElementsAre(1,2,4));
  line->getDocIds(2,docids);
  EXPECT_THAT(docids,ElementsAre(1,3,7));
  line->addDocument(1,3);
  line->commit(out);
  line->getDocIds(1,docids);
  EXPECT_THAT(docids,ElementsAre(3));
  line->getDocIds(2,docids);
  EXPECT_THAT(docids,ElementsAre(1,3,7));
  line->addDocument(1,1);
  line->commit(out);
  line->getDocIds(1,docids);
  EXPECT_THAT(docids,ElementsAre(1,3));
  line->getDocIds(2,docids);
  EXPECT_THAT(docids,ElementsAre(1,3,7));
  line->addDocument(1,9);
  line->commit(out);
  line->getDocIds(1,docids);
  EXPECT_THAT(docids,ElementsAre(1,3,9));
  line->getDocIds(2,docids);
  EXPECT_THAT(docids,ElementsAre(1,3,7));
  line->addDocument(5,9);
  line->commit(out);
  line->getDocIds(1,docids);
  EXPECT_THAT(docids,ElementsAre(1,3,9));
  line->getDocIds(2,docids);
  EXPECT_THAT(docids,ElementsAre(1,3,7));
  line->getDocIds(5,docids);
  EXPECT_THAT(docids,ElementsAre(9));
  delete[] out;
  delete[] in;
}

//cout << *line;
//cout <<"---------------"<<endl;


// TEST(PostLineTest,BigPostLineTest){
//   unsigned char* in = new unsigned char[4096];
//   unsigned char* out = new unsigned char[4096];
//   vector<uint32_t> deltas;
//   vector<uint32_t> docids;
//   memset(in,0,4096);
//   PostLineCodec* codec = new VarIntCodec();
//   PostLine* line = new PostLine(codec,4096*5);
//   // line->load(in);
//   // for (size_t doc_type=1;doc_type<4;doc_type++){
//   //   for (size_t doc_id=1;doc_id<16;doc_id++){
//   //     line->addDocument(doc_type,doc_id);
//   //     line->commit(in);
//   //   }
//   // }
//   // cout << *line << endl;
//   // cout <<"---------------"<<endl;
//   // for (size_t doc_type=3;doc_type>0;doc_type--){
//   //   for (size_t doc_id=1;doc_id<16;doc_id++){
//   //     line->addDocument(doc_type,doc_id);
//   //     line->commit(in);
//   //   }
//   //   cout << *line;
//   //   cout <<"---------------"<<endl;
//   // }
//   memset(in,0,4096);
//   line->load(in);
//   for (size_t doc_type=3;doc_type>0;doc_type--){
//     for (size_t doc_id=1;doc_id<16;doc_id++){
//       line->addDocument(doc_type,doc_id);
//       line->commit(in);
//       cout << *line;
//       cout <<"---------------"<<endl;
//     }
//   }
//   // length=line->getLength(1);
//   // EXPECT_EQ(1U,length);
//   // length=line->getLength();
//   // EXPECT_EQ(4U,length);
//   // line->commit(out);
//   // EXPECT_EQ(out[0],1);
//   // EXPECT_EQ(out[1],1);
//   // EXPECT_EQ(out[2],0);
//   // EXPECT_EQ(out[3],1);
//   // line->load(out);
//   // line->getDeltas(1,deltas);
//   // EXPECT_THAT(deltas,ElementsAre(1));
//   // line->getDocIds(1,docids);
//   // EXPECT_THAT(deltas,ElementsAre(1));
//   // line->addDocument(1,2);
//   // line->commit(out);
//   // line->getDeltas(1,deltas);
//   // EXPECT_THAT(deltas,ElementsAre(1,1));
//   // line->getDocIds(1,docids);
//   // EXPECT_THAT(docids,ElementsAre(1,2));
//   delete[] out;
//   delete[] in;
// }


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}