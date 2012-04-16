#include <tests.h>
#include <cstdlib>
#include <document.h>

typedef shared_ptr<PostLineCodec> PostLineCodecPtr;

class SearchMapTest : public Test{
  public:
     static vector<uint32_t> input;
  
     static void SetUpTestCase(){
       input.clear();
       srand(time(NULL));
       input.reserve(10000000);
       for (size_t i=0;i<10000000;i++){
         input.push_back(rand()%20+1);
         input.push_back(rand()%10000000+1);
       }
     }
};

vector<uint32_t> SearchMapTest::input;

TEST_F(SearchMapTest,SlowPrefetchMapTest){
  search_t results;
  // results.rehash(20000);
  vector<uint32_t>::const_iterator it=SearchMapTest::input.begin(),ite=SearchMapTest::input.end();
  DocTally* previous=&results[DocPair(*it++,*it++)];
  for (;it!=ite;){
    DocTally* current=previous;
    previous=&results[DocPair(*it++,*it++)];
    __builtin_prefetch (previous, 1, 3);
    current->count++;
  }
  // size_t count=0;
  // for (size_t i=0;i<results.bucket_count();i++){
  //   if (results.bucket_size(i)>1){
  //     count++;
  //     // cout << i << ":" << tallies.bucket_size(i) << endl; 
  //   }
  // }
  // cout << count << " collisons" << endl;
}

TEST_F(SearchMapTest,SlowSearchMapTest){
  search_t results;
  results.rehash(50000);
  for (vector<uint32_t>::const_iterator it=SearchMapTest::input.begin(),ite=SearchMapTest::input.end();it!=ite;){
    DocPair pair(*it++,*it++);
    DocTally* tally=&results[pair];
    tally->count++;
    // if ((i%1000)==0){
    //   cout << "Loop: " << i << " Load Factor: " << results.load_factor() << " Count: " << results.size() <<endl; 
    // }
  }
  // for (search_t::const_iterator it=results.begin(),ite=results.end();it!=ite;++it){
  //   size_t bucket=results.bucket(it->first);
  //   cout << "Bucket: " << bucket << "/" << results.bucket_count() <<" Doctype: " << it->first.doc_type << " Docid: " << it->first.doc_id << " Bucket size: " << results.bucket_size(bucket);
  //   cout << " Key: " << (void*)&it->first  << " Value : " << (void*)&it->second << endl;
  // }
}

typedef struct
{
  inline bool operator() (const DocPair& x, const DocPair &y) const { 
    // return ~((uint64_t(x.doc_type)<<32|x.doc_id)^(uint64_t(y.doc_type)<<32|y.doc_id));
    // return ~((x.doc_id^y.doc_id)&&(x.doc_type^y.doc_type));
    return (x.doc_id==y.doc_id)&(x.doc_type==y.doc_type);
    // return (x.doc_type==y.doc_type)&&(x.doc_id==y.doc_id);
  }
} DocPairEq2;

typedef struct{
  inline size_t operator() (const DocPair& k) const {
    return (uint64_t(k.doc_type)*73856093)^(uint64_t(k.doc_id)*19349663);
    // return (static_cast<uint64_t>(k.doc_type)<<32)|k.doc_id;
    // return (uint64_t(k.doc_type)<<32|k.doc_id);
  }
} DocPairHash2;

typedef unordered_set<DocPair,DocPairHash2,DocPairEq2> searchset_t;
// typedef unordered_map<DocPair,DocTally,DocPairHash2,DocPairEq2> pooled_search_t;
typedef unordered_map<DocPair,DocTally,DocPairHash2,DocPairEq2,boost::fast_pool_allocator<pair<DocPair,DocTally> > > pooled_search_t;

TEST_F(SearchMapTest,SlowPooledSearchMapTest){
  pooled_search_t results;
  vector<uint32_t>::const_iterator it=SearchMapTest::input.begin(),ite=SearchMapTest::input.end();
  DocTally* previous=&results[DocPair(*it++,*it++)];
  for (;it!=ite;){
    DocTally* current=previous;
    previous=&results[DocPair(*it++,*it++)];
    __builtin_prefetch (previous, 1, 3);
    current->count++;
  }
  // results.rehash(50000);
  // size_t count=0;
  // for (vector<uint32_t>::const_iterator it=SearchMapTest::input.begin(),ite=SearchMapTest::input.end();it!=ite;){
  //   DocPair pair(*it++,*it++);
  //   DocTally* tally=&results[pair];
  //   tally->count++;
  //   // if(count%1000==0){
  //   //   cout << results.bucket_count() <<":" << results.load_factor() <<endl;
  //   // }
  //   // count++;
  // }
  // vector<void*> addresses;
  // for(search_t::const_iterator it=results.begin(),ite=results.end();it!=ite;++it){
  //   addresses.push_back((void*)&it->first);
  //   addresses.push_back((void*)&it->second);
  // }
  // sort(addresses.begin(),addresses.end());
  // for(vector<void*>::const_iterator it=addresses.begin(),ite=addresses.end();it!=ite;++it){
  //   cout << *it << endl;
  // }
}

class CodecBenchmarkTest : public TestWithParam<PostLineCodecPtr>{
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

TEST_P(CodecBenchmarkTest,SlowCodecSpeedTest){
  timeval t1, t2;
  double elapsedTime;
  PostLineCodecPtr codec = GetParam();
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

const static vector<PostLineCodecPtr> getCodecs(){
  vector<PostLineCodecPtr> codecs;
  codecs.push_back(PostLineCodecPtr(new VarIntCodec()));
  codecs.push_back(PostLineCodecPtr(new GroupVarIntCodec()));
  return codecs;
}

INSTANTIATE_TEST_CASE_P(CodecBenchmarkTests,
                        CodecBenchmarkTest,
                        ValuesIn(getCodecs()));
