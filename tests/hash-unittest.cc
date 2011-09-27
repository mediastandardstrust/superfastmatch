#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdint.h>
#include <sys/time.h>
#include <cctype>

using namespace testing;
using namespace std;
using namespace std::tr1;

static void MurmurHash2(const string& text, const size_t window_size,const size_t whitespace_threshold, vector<uint32_t>& hashes) {
  hashes.clear();
  hashes.reserve(text.size());
  const uint64_t mul = 0xc6a4a7935bd1e995ULL;
  const int32_t rtt = 47;
  for (size_t i=0;i<text.size()-window_size+1;i++){
    size_t size=window_size;
    const unsigned char* rp = (const unsigned char*)text.data()+i;
    uint64_t hash = 19780211ULL ^ (window_size * mul);
    while (size >= sizeof(uint64_t)) {
      uint64_t num = ((uint64_t)rp[0] << 0) | ((uint64_t)rp[1] << 8) |
        ((uint64_t)rp[2] << 16) | ((uint64_t)rp[3] << 24) |
        ((uint64_t)rp[4] << 32) | ((uint64_t)rp[5] << 40) |
        ((uint64_t)rp[6] << 48) | ((uint64_t)rp[7] << 56);
      num *= mul;
      num ^= num >> rtt;
      num *= mul;
      hash *= mul;
      hash ^= num;
      rp += sizeof(uint64_t);
      size -= sizeof(uint64_t);
    }
    switch (size) {
    case 7: hash ^= (uint64_t)rp[6] << 48;
    case 6: hash ^= (uint64_t)rp[5] << 40;
    case 5: hash ^= (uint64_t)rp[4] << 32;
    case 4: hash ^= (uint64_t)rp[3] << 24;
    case 3: hash ^= (uint64_t)rp[2] << 16;
    case 2: hash ^= (uint64_t)rp[1] << 8;
    case 1: hash ^= (uint64_t)rp[0];
      hash *= mul;
    };
    hash ^= hash >> rtt;
    hash *= mul;
    hash ^= hash >> rtt;
    hashes.push_back(hash); 
  }
}

// #define IsWhiteSpace(ch) not((unsigned(ch-97)<=25)|(unsigned(ch-48)<=9)|(ch==92))

// 0123456789012345678901234567890123456789012345678901234567890123
// /0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmn
// 0111111111100000001111111111111111111111111101000000000000000000
#define IsWhiteSpace(ch)((1ULL<<(ch-47))&0x2FFFFFFC07FE)==0


// Taken from http://www.azillionmonkeys.com/qed/asmexample.html
static inline uint64_t UpperCase(uint64_t upper){
  uint64_t u1 = 0x8080808080808080ul | upper;
  uint64_t u2 = u1-0x6161616161616161ul;
  uint64_t u3 = ~(u1-0x7b7b7b7b7b7b7b7bul);
  uint64_t u4 = (u2 & u3) & (~upper & 0x8080808080808080ul);
  return upper-=(u4 >> 2); 
}

static inline uint64_t fmix ( uint64_t h )
{
  return h*= 0xff51afd7ed558ccd;
}

static void UpperCaseRabinKarp(const string& text,const size_t window_size,const size_t whitespace_threshold,vector<uint32_t>& hashes){
  const uint64_t base=37;
  const size_t limit=text.size()-window_size+1;
  uint64_t* f=(uint64_t*)text.data();
  uint64_t* b=(uint64_t*)(text.data()+window_size);
  hashes.clear();
  hashes.resize(limit);
  uint64_t highBase=1;
  uint64_t hash=0;
  uint64_t whitespace=0;
  uint64_t whitespaceHash=0;
  for (size_t i=0;i<window_size;i++){
    highBase=(i==0)?1:highBase*base;
    char c=toupper(text[window_size-i-1]);
    hash+=c*highBase;
    whitespaceHash+=32*highBase;
    whitespace+=IsWhiteSpace(c);
  }
  const uint64_t high=highBase;
  hashes[0]=fmix(hash);
  whitespaceHash=fmix(whitespaceHash);
  vector<uint32_t>::iterator it=hashes.begin()+1;
  for (size_t i=(limit/8);i>0;i--){
    uint64_t uf=UpperCase(*f++);
    uint64_t ub=UpperCase(*b++);
    for (size_t i=8;i>0;i--){
      uint32_t front=uf&0xFF;
      uint32_t back=ub&0xFF;
      uf>>=8;
      ub>>=8;
      whitespace-=IsWhiteSpace(front);
      whitespace+=IsWhiteSpace(back);
      uint32_t mask=((whitespace<whitespace_threshold)-1);
      hash-=front*high;
      hash*=base;
      hash+=back;
      *it=(fmix(hash)&~mask)|(whitespaceHash&mask);
      // cout << text.substr(it-hashes.begin(),window_size) << ":" << char(front) << ":" << int(IsWhiteSpace(front)) << ":" << char(back) << ":" << int(IsWhiteSpace(back)) << ":" << hash  << ":" << *it << ":" << mask << ":" << whitespace << endl; 
      ++it;
    }
  }
  for (size_t i=(limit/8)*8;i<limit;i++){
    char front=toupper(text[i]);
    char back=toupper(text[i+window_size]);
    hash-=front*high;
    hash*=base;
    hash+=back;
    whitespace+=IsWhiteSpace(front);
    whitespace-=IsWhiteSpace(back);
    *it=(whitespace<whitespace_threshold)?(fmix(hash)):whitespaceHash;
    // cout << text.substr(it-hashes.begin(),window_size) << ":" << char(front) << ":" << int(IsWhiteSpace(front)) << ":" << char(back) << ":" << int(IsWhiteSpace(back)) << ":" << hash  << ":" << *it << ":" << whitespace << endl; 
    ++it;
  }
}

static void RabinKarp(const string& text,const size_t window_size,const size_t whitespace_threshold,vector<uint32_t>& hashes){
  const uint64_t base=37;
  const size_t limit=text.size()-window_size+1;
  string::const_iterator front=text.begin();
  hashes.clear();
  hashes.resize(limit);
  uint64_t highBase=1;
  uint64_t hash=0;
  for (size_t i=0;i<window_size;i++){
    highBase=(i==0)?1:highBase*base;
    hash+=front[window_size-i-1]*highBase;
  }
  hashes[0]=fmix(hash);
  for (vector<uint32_t>::iterator it=hashes.begin()+1,ite=hashes.end();it!=ite;++it){
    hash-=(*front)*highBase;
    hash*=base;
    hash+=front[window_size];
    ++front;
    *it=fmix(hash);
    // cout << "Next:  " << text.substr(it-hashes.begin(),window_size) << ":" << *(front-1) << ":" << front[window_size] << ":" << hash << endl;
  }
}

// Testing apparatus

typedef void(*HasherFun)(const string&,const size_t,const size_t,vector<uint32_t>&);

class Document{
private:
  string text_;
  unordered_map<size_t,size_t> uniques_;
public:
  Document(const char* filename){
    ifstream file(filename);
    stringstream buffer;
    buffer << file.rdbuf();
    text_=buffer.str();
    EXPECT_NE(0U,text_.size());
  }
  
  string& getText(){
    return text_;
  }
  
  size_t getUniques(const size_t window_size){
    if (uniques_.find(window_size)==uniques_.end()){
      unordered_set<string> uniques;
      for (size_t i=0;i<text_.size()-window_size+1;i++){
        uniques.insert(text_.substr(i,window_size));
      }
      uniques_[window_size]=uniques.size();
    }
    return uniques_[window_size];
  }
};

class HashTest : public TestWithParam<tuple<tuple<string,HasherFun>,Document*,size_t> > {
public:
  HasherFun hasher_;
  Document* doc_;
  size_t window_size_;

  virtual void SetUp(){
    RecordProperty("Hasher",get<0>(get<0>(GetParam())).c_str());
    hasher_=get<1>(get<0>(GetParam()));
    doc_=get<1>(GetParam());
    window_size_=get<2>(GetParam());
  }
};

class CorrectnessTest : public TestWithParam<tuple<string,HasherFun> > {};

TEST_P(HashTest,SlowCollisionTest){
  vector<uint32_t>hashes;
  vector<uint32_t>distribution(16);
  const uint32_t divisor=(1<<28);
  hasher_(doc_->getText(),window_size_,40,hashes);
  unordered_set<uint32_t> unique_hashes(hashes.begin(),hashes.end());
  size_t uniques=doc_->getUniques(window_size_);
  RecordProperty("Window size",window_size_);
  RecordProperty("Windows",doc_->getText().size()-window_size_+1);
  RecordProperty("Uniques",uniques);
  RecordProperty("Unique Hashes",unique_hashes.size());
  stringstream s;
  s <<double(uniques-unique_hashes.size())/uniques;
  RecordProperty("Collisions",s.str().c_str());
  for(unordered_set<uint32_t>::const_iterator it=unique_hashes.begin(),ite=unique_hashes.end();it!=ite;++it){
    distribution[*it/divisor]++;
  }
  double min=1E+37;
  double max=0;
  stringstream t;
  for(vector<uint32_t>::const_iterator it=distribution.begin(),ite=distribution.end();it!=ite;++it){
    double count=double(*it)/unique_hashes.size();
    min=std::min(min,count);
    max=std::max(max,count);
    t << setprecision(4) << count << ":";
  }
  t << " Range: " << max-min;
  RecordProperty("Distribution",t.str().c_str());
}

TEST_P(HashTest,SpeedTest){
  string text(doc_->getText());
  timeval t1, t2;
  double elapsedTime;
  gettimeofday(&t1, NULL);
  vector<uint32_t>hashes;
  hasher_(text,window_size_,6,hashes);
  gettimeofday(&t2, NULL);
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; 
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
  RecordProperty("Benchmark",elapsedTime);
  EXPECT_EQ(hashes.size(),text.size()-window_size_+1);
}

TEST_P(CorrectnessTest,ShortTest){
  HasherFun hasher=get<1>(GetParam());
  vector<uint32_t>hashes1,hashes2,hashes3,hashes4,hashes5;
  hasher("this is a test this is a test!!",11,6,hashes1);
  hasher("test gobbledegook gobbledegook gobbledegook gobbledegook gobbledegook gobbledegook gobbledegook gobbledegook gobbledegook gobbledegook gobbledegook gobbledegook gobbledegook gobbledegook in the test",4,6,hashes2);
  hasher("test this is",4,6,hashes3);
  hasher("          ",4,6,hashes4);
  hasher("test          test",4,6,hashes5);
  EXPECT_EQ(21U,hashes1.size());
  EXPECT_EQ(195U,hashes2.size());
  EXPECT_EQ(9U,hashes3.size());
  EXPECT_EQ(hashes2.back(),hashes2.front());
  EXPECT_EQ(hashes2[7],hashes2[20]);
  EXPECT_EQ(14U,count(hashes2.begin(),hashes2.end(),hashes2[7]));
  EXPECT_EQ(hashes2.back(),hashes3.front());
  EXPECT_EQ(hashes4[0],hashes5[6]);
  RecordProperty("Hasher",get<0>(GetParam()).c_str());
}

const static vector<tuple<string,HasherFun> > getHashers(){
  vector<tuple<string,HasherFun> > hashers;
  hashers.push_back(make_tuple<string,HasherFun>("Rabin Karp",&RabinKarp));
  hashers.push_back(make_tuple<string,HasherFun>("Upper Case Rabin Karp",&UpperCaseRabinKarp));
  hashers.push_back(make_tuple<string,HasherFun>("Murmur Hash 2",&MurmurHash2));
  return hashers;
}

const static vector<Document*> getDocs(){
  vector<Document*> docs;
  // docs.push_back(new Document("fixtures/congressional-record/2001-12.txt"));
  docs.push_back(new Document("fixtures/gutenberg/bible.txt"));
  return docs;
}

INSTANTIATE_TEST_CASE_P(RangeTests,
                        HashTest,
                        Combine(ValuesIn(getHashers()),ValuesIn(getDocs()),Values(10,20,100)));

INSTANTIATE_TEST_CASE_P(CorrectnessTests,
                        CorrectnessTest,
                        ValuesIn(getHashers()));
