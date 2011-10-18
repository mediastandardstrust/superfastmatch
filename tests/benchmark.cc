#include <tests.h>
#include <cstdlib>
#include <document.h>

TEST(BenchmarkTest,SearchMapTest){
  search_t results;
  for (size_t i=0;i<10000000;i++){
    DocPair pair(rand()%2+1,rand()%10000+1);
    DocTally* tally=&results[pair];
    tally->count++;
    // if ((i%1000)==0){
    //   cout << "Loop: " << i << " Load Factor: " << results.load_factor() << " Count: " << results.size() <<endl; 
    // }
  }
  // for (search_t::const_iterator it=results.begin(),ite=results.end();it!=ite;++it){
  //   size_t bucket=results.bucket(it->first);
  //   cout << "Bucket: " << bucket << "/" << results.bucket_count() <<" Doctype: " << it->first.doc_type << " Docid: " << it->first.doc_id << " Bucket size: " << results.bucket_size(bucket) << endl;
  // }
}

class CodecBenchmarkTest : public TestWithParam<PostLineCodec*>{
public:
  static vector<uint32_t> input;
  
  static void SetUpTestCase(){
    input.clear();
    srand(time(NULL));
    input.reserve(100000008);
    size_t length=100000000+(rand()%8);
    for (size_t i=0;i<length;i++){
      input.push_back(rand()%17000000+1);
    }
  }  
};

vector<uint32_t> CodecBenchmarkTest::input;

TEST_P(CodecBenchmarkTest,CodecSpeedTest){
  timeval t1, t2;
  double elapsedTime;
  PostLineCodec* codec = GetParam();
  vector<uint32_t> output;
  output.reserve(100000000);
  unsigned char* out = new unsigned char[100000000*5];
  gettimeofday(&t1, NULL);
  size_t length=codec->encodeSection(CodecBenchmarkTest::input,out);
  gettimeofday(&t2, NULL);
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; 
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
  RecordProperty("Encode",elapsedTime);
  RecordProperty("Length",length);
  EXPECT_NE(0U,length);
  gettimeofday(&t1, NULL);
  EXPECT_EQ(length,codec->decodeSection(out,length,output,true));
  gettimeofday(&t2, NULL);
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; 
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
  RecordProperty("Decode",elapsedTime);
  EXPECT_EQ(input.front(),output.front());
  EXPECT_EQ(input.back(),output.back());
  EXPECT_EQ(CodecBenchmarkTest::input.size(),output.size());
  delete[] out;
}

const static vector<PostLineCodec*> getCodecs(){
  vector<PostLineCodec*> codecs;
  codecs.push_back(new VarIntCodec());
  codecs.push_back(new GroupVarIntCodec());
  return codecs;
}

INSTANTIATE_TEST_CASE_P(CodecBenchmarkTests,
                        CodecBenchmarkTest,
                        ValuesIn(getCodecs()));
