#include <tests.h>
#include <cstdlib>
#include <document.h>

#include <ext/pool_allocator.h>
#include <ext/bitmap_allocator.h>
#include <boost/pool/pool_alloc.hpp>

using namespace boost;

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

static uint32_t nearest_pow (uint32_t num)
{
    uint32_t n = 1;
    while (n < num)
        n <<= 1;
    return n;
}
// 
// class Tally : public intrusive::unordered_set_base_hook<>
// {
//   public:
//     uint32_t doc_type;
//     uint32_t doc_id;
//     uint64_t previous;
//     uint32_t count;
// 
//     Tally():
//     doc_type(0),
//     doc_id(0),
//     previous(0),
//     count(0)
//     {}
// 
//     inline friend bool operator== (const Tally &a, const Tally &b){
//       return (a.doc_type==b.doc_type)&&(a.doc_id==b.doc_id);
//     }
// 
//     inline friend std::size_t hash_value(const Tally &value){
//       // return (uint64_t(value.doc_type)*0xff51afd7ed558ccd)^(value.doc_id*8745648375);
//       std::size_t seed = 0;
//       boost::hash_combine(seed, value.doc_type*8745648375);
//       // boost::hash_combine(seed, value.doc_type*0xc2b2ae35);
//       boost::hash_combine(seed, value.doc_id);
//       return seed;
//     }
// };
// 
// typedef intrusive::unordered_set<Tally,intrusive::constant_time_size<false>,intrusive::power_2_buckets<true> > TallySet;
// //typedef intrusive::unordered_set<Tally,intrusive::constant_time_size<false> > TallySet;
// 
// 
// TEST_F(SearchMapTest,InstrusiveSetTest){
//   vector<Tally> storage;
//   size_t length=nearest_pow(100000);
//   storage.reserve(length);
//   TallySet::bucket_type buckets[length];
//   TallySet tallies(TallySet::bucket_traits(buckets,length));
//   vector<uint32_t>::const_iterator it=SearchMapTest::input.begin(),ite=SearchMapTest::input.end();
//   vector<pair<Tally,TallySet::size_type> > batch;
//   // batch.resize(1000);
//   // for (;it!=ite;){
//   //   for (size_t i=0;i<1000;i++){
//   //     batch[i].first.doc_type=*it++;
//   //     batch[i].first.doc_id=*it++;
//   //     batch[i].second=tallies.bucket(batch[i].first);
//   //     __builtin_prefetch(&buckets[batch[i].second],0,0);
//   //   }
//   //   for (size_t i=0;i<1000;i++){
//   //     __builtin_prefetch(((void*)(buckets+batch[i].second)),0,0);
//   //   }
//   //   for (size_t i=0;i<1000;i++){
//   //     TallySet::iterator tally_it=tallies.find(batch[i].first);
//   //     if(likely(tally_it!=tallies.end())){
//   //       tally_it->count++;
//   //       // cout << storage.capacity() << ":" <<tally.doc_type << ":" << tally.doc_id << ":" << tally_it->count << ":" << sizeof(*tally_it) << ":" << &*tally_it << endl;
//   //     }else{
//   //       storage.push_back(batch[i].first);
//   //       tallies.insert(storage.back());
//   //     }      
//   //   }
//   // }
//   Tally tally;
//   for (;it!=ite;){
//     vector<Tally> batch;
//     tally.doc_type=*it++;
//     tally.doc_id=*it++;
//   
//     TallySet::iterator tally_it=tallies.find(tally);
//     if(likely(tally_it!=tallies.end())){
//       tally_it->count++;
//       // cout << storage.capacity() << ":" <<tally.doc_type << ":" << tally.doc_id << ":" << tally_it->count << ":" << sizeof(*tally_it) << ":" << &*tally_it << endl;
//     }else{
//       storage.push_back(tally);
//       tallies.insert(storage.back());
//     }
//   }
//   size_t count=0;
//   for (size_t i=0;i<length;i++){
//     if (tallies.bucket_size(i)>1){
//       count++;
//       // cout << i << ":" << tallies.bucket_size(i) << endl; 
//     }
//   }
//   cout << count << " collisons" << endl; 
// }

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

TEST_P(CodecBenchmarkTest,SlowCodecSpeedTest){
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
