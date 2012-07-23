#ifndef _SFMCOMMON_H                       // duplication check
#define _SFMCOMMON_H

#ifdef _MSC_VER
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

#include <iostream>
#include <algorithm>
#include <functional>
#include <vector>
#include <deque>
#include <queue>
#include <bitset>
#include <map>
#include <sstream>
#include <cstdlib>
#include <iomanip>
#include <string>
#include <limits>
#include <kcutil.h>
#include <ktutil.h>
#include <tr1/memory>
#include <tr1/unordered_set>
#include <ctemplate/template.h>
#include <emmintrin.h>

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

using namespace ctemplate;
using namespace std;
using namespace std::tr1;
namespace superfastmatch{
  namespace kt=kyototycoon;
  namespace kc=kyotocabinet;

  // Global Forward Declarations
  class Document;
  class DocumentQuery;
  class Association;
  class Command;
  class Instrument;

  // Global Typedefs
  typedef shared_ptr<Document> DocumentPtr;
  typedef shared_ptr<DocumentQuery> DocumentQueryPtr;
  typedef shared_ptr<Association> AssociationPtr;
  typedef shared_ptr<Command> CommandPtr;

  // Global consts
  const uint64_t MAX_HASH=(1L<<32)-1;
  const unsigned char WHITESPACE=47;
  
  // Unicode helpers
  #define isutf(c) (((c)&0xC0)!=0x80)
  inline void u8_inc(const char *s, size_t *i)
  {
      (void)(isutf(s[++(*i)]) || isutf(s[++(*i)]) ||
              isutf(s[++(*i)]) || ++(*i));
  }
  
  //Global utility functions
    
  inline bool needsAllocation(const size_t incoming, const size_t outgoing, const size_t block_size, size_t& block){
    block=(((outgoing-1)/block_size)+1)*block_size;
    return block!=(((incoming-1)/block_size)+1)*block_size;
  }
    
  #define likely(x) __builtin_expect(!!(x), 1)
  #define unlikely(x) __builtin_expect(!!(x), 0)
  
  #define IsWhiteSpace(ch)((ch<=WHITESPACE)|(((1ULL<<(ch-47))&0x2FFFFFFC07FE)==0))
  
  inline bool notAlphaNumeric(char c){
    return IsWhiteSpace(c);
    // return std::isalnum(c)==0;
  }
  
  // Taken from http://www.azillionmonkeys.com/qed/asmexample.html
  inline uint64_t UpperCase(uint64_t upper){
    uint64_t u1 = 0x8080808080808080ul | upper;
    uint64_t u2 = u1-0x6161616161616161ul;
    uint64_t u3 = ~(u1-0x7b7b7b7b7b7b7b7bul);
    uint64_t u4 = (u2 & u3) & (~upper & 0x8080808080808080ul);
    return upper-=(u4 >> 2); 
  }
  
  // Taken from http://www.azillionmonkeys.com/qed/asmexample.html
  inline uint64_t Normalise(uint64_t upper){
    uint64_t u1 = 0x8080808080808080ul | upper;
    uint64_t u2 = u1-0x6161616161616161ul;
    uint64_t u3 = ~(u1-0x7b7b7b7b7b7b7b7bul);
    uint64_t u4 = (u2 & u3) & (~upper & 0x8080808080808080ul);
    upper-=(u4 >> 2);
    const __m64 ceiling=__m64(0x2F2F2F2F2F2F2F2Ful);
    __m64 u=__m64(upper);
    __m64 m=_mm_max_pu8(u,ceiling);
    return uint64_t(m);
  }
  
  inline uint64_t fmix ( uint64_t h )
  {
    // h ^= h >> 16;
    // h *= 0x85ebca6b;
    // h ^= h >> 13;
    // h *= 0xc2b2ae35;
    // h ^= h >> 16;
    // return h;
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccd;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53;
    h ^= h >> 33;
    return h;
    return h*= 0xff51afd7ed558ccd;
  }

  inline uint32_t WhiteSpaceHash(const size_t window_size){
    const uint64_t base=37;
    uint64_t whitespace=0;
    uint64_t highBase=1;
    for (size_t i=0;i<window_size;i++){
      highBase=(i==0)?1:highBase*base;
      whitespace+=WHITESPACE*highBase;
    }
    return fmix(whitespace);
  }

  inline void UpperCaseRabinKarp(const string& text,const size_t window_size,const size_t whitespace_threshold,vector<uint32_t>& hashes){
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
      uint64_t c=Normalise(text[window_size-i-1])&0xFF;
      hash+=c*highBase;
      whitespaceHash+=32*highBase;
      whitespace+=(c==WHITESPACE);
    }
    const uint64_t high=highBase;
    whitespaceHash=fmix(whitespaceHash);
    hashes[0]=(whitespace<whitespace_threshold)?(fmix(hash)):whitespaceHash;
    // cout << text.substr(0,window_size) << ":" << text[0] << ":" << int(IsWhiteSpace(text[0])) << ":" << char(text[window_size]) << ":" << int(IsWhiteSpace(text[window_size])) << ":" << hash  << ":" << hashes[0] << ":" << whitespace << endl; 
    vector<uint32_t>::iterator it=hashes.begin()+1;
    vector<uint32_t>::iterator ite=hashes.end();
    size_t split=(limit-1)/8;
    for (size_t i=split;i>0;i--){
      uint64_t uf=Normalise(*f++);
      uint64_t ub=Normalise(*b++);
      for (size_t j=8;j>0;j--){
        uint32_t front=uf&0xFF;
        uint32_t back=ub&0xFF;
        uf>>=8;
        ub>>=8;
        whitespace-=(front==WHITESPACE);
        whitespace+=(back==WHITESPACE);
        uint32_t mask=((whitespace<whitespace_threshold)-1);
        hash-=front*high;
        hash*=base;
        hash+=back;
        *it=(fmix(hash)&~mask)|(whitespaceHash&mask);
        // cout << text.substr(it-hashes.begin(),window_size) << ":" << char(front) << ":" << int(IsWhiteSpace(front)) << ":" << char(back) << ":" << int(IsWhiteSpace(back)) << ":" << hash  << ":" << *it << ":" << mask << ":" << whitespace << endl; 
        assert(it!=ite);
        ++it;
      }
    }
    for (size_t i=split*8+1;i<limit;i++){
      uint64_t front=Normalise(text[i-1])&0xFF;
      uint64_t back=Normalise(text[i+window_size-1])&0xFF;
      hash-=front*high;
      hash*=base;
      hash+=back;
      whitespace-=(front==WHITESPACE);
      whitespace+=(back==WHITESPACE);
      *it=(whitespace<whitespace_threshold)?(fmix(hash)):whitespaceHash;
      // cout << text.substr(i,window_size) << ":" << char(front) << ":" << int(IsWhiteSpace(front)) << ":" << char(back) << ":" << int(IsWhiteSpace(back)) << ":" << hash  << ":" << *it << ":" << whitespace << "!" << endl; 
      assert(it!=ite);
      ++it;
    }
  }
  
  inline uint32_t hashmurmur(const void* buf, size_t size) {
    const uint64_t mul = 0xc6a4a7935bd1e995ULL;
    const int32_t rtt = 47;
    uint64_t hash = 19780211ULL ^ (size * mul);
    const unsigned char* rp = (const unsigned char*)buf;
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
    return hash;
  }
  
  template <class C> void FreeClear( C & cntr ) {
    for ( typename C::iterator it = cntr.begin(); it != cntr.end(); ++it ) {
      if (*it!=0){
        delete *it;
        *it=0;
      }
    }
    cntr.clear();
  }
  
  inline bool isNumeric(const string& input)
  {
    char* end = 0;
    std::strtod(input.c_str(), &end);
    return end != 0 && *end == 0;
  }
  
  
  inline string toString(uint64_t number){ 
    stringstream s;
    s << number;
    return s.str();
  }
  
  inline void Replace(std::string& str, const std::string& oldStr, const std::string& newStr)
  {
    size_t pos = 0;
    while((pos = str.find(oldStr, pos)) != std::string::npos)
    {
       str.replace(pos, oldStr.length(), newStr);
       pos += newStr.length();
    }
  }
  
  inline string padIfNumber(const string& input){
    if (isNumeric(input)){
      stringstream s;
      s << setfill('0') <<setw(10) << kc::atoi(input.c_str());
      return s.str();
    }
    return input;
  }
  
  inline void fillMetaDictionary(const string& key,const string& value,TemplateDictionary* dict,set<string>& metadata){
    if (value.length()>0){
      TemplateDictionary* metaDict=dict->AddSectionDictionary("META");
      bool isNumber=isNumeric(value);
      TemplateDictionary* valDict;
      if(isNumber){
        valDict=metaDict->AddSectionDictionary("NUMBER");
      }else{
        valDict=metaDict->AddSectionDictionary("STRING");
      }
      metaDict->SetValue("KEY",key);
      valDict->SetValue("VALUE",value);
      metadata.insert(key);
    }
  }
    
  template <typename T, typename U>
  class create_map
  {
  private:
      std::map<T, U> m_map;
  public:
      create_map(const T& key, const U& val)
      {
          m_map[key] = val;
      }

      create_map<T, U>& operator()(const T& key, const U& val)
      {
          m_map[key] = val;
          return *this;
      }

      operator std::map<T, U>()
      {
          return m_map;
      }
  };
  
  template <typename T>
  class create_set
  {
  public:
      create_set(const T& val)
      {
          set.insert(val);
      }
      create_set& operator()(T val)
      {
          set.insert(val);
          return *this;
      }
      operator std::set<T>()
      {
          return set;
      }
  private:
      std::set<T> set;
  };
  
  template <typename T>
  class create_vector
  {
  public:
      create_vector(const T& val)
      {
          vector.push_back(val);
      }
      create_vector& operator()(T val)
      {
          vector.push_back(val);
          return *this;
      }
      operator std::vector<T>()
      {
          return vector;
      }
  private:
      std::vector<T> vector;
  };
}

#endif
