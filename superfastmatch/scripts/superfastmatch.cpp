#define LUA_SUPERFASTMATCH_VERSION	"0.4"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include <utility>
#include <iostream>
#include <deque>
#include <string>
#include <bitset>
#include <vector>
#include <algorithm>
#include <iterator>
// #include <boost/unordered_set.hpp>
// #include <boost/unordered_map.hpp>
// #include <tr1/unordered_set>
// #include <tr1/unordered_map>
#include <google/dense_hash_set>
#include <google/dense_hash_map>

#include "MurmurHash3.h"
#include "kcutil.h"

// using namespace std;
// using namespace std::tr1;
using namespace kyotocabinet;

#ifdef _MSC_VER
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

typedef uint32_t hash_t;
typedef uint32_t position_t;
typedef std::vector<hash_t> hashes_vector;
typedef std::tr1::unordered_set<position_t> positions_set;
typedef std::tr1::unordered_set<hash_t> hashes_set;   
typedef std::tr1::unordered_map<hash_t,std::tr1::unordered_set<position_t> > matches_map;


const uint32_t BLOOM_BITS=24;
const uint32_t BLOOM_MASK=(((u_int32_t)1L<<BLOOM_BITS)-1);
const uint32_t BLOOM_SIZE=(1L<<BLOOM_BITS);
typedef std::bitset<BLOOM_SIZE> bloom_bitset;

struct Result{
   position_t left;
   position_t right;
   position_t length;
   Result(position_t left,position_t right,position_t length) : left(left), right(right), length(length){}
   friend std::ostream& operator<< (std::ostream &o, const Result &r)
   {
      return o << "left: " << r.left << " right: " << r.right << " length: " << r.length << std::endl;
   }
};

bool result_sorter(Result const& lhs,Result const& rhs ){
   if (lhs.length!=rhs.length)
      return lhs.length > rhs.length;
   if (lhs.left!=rhs.left)
      return lhs.left < rhs.left;
   return lhs.right < rhs.right;
}

struct Match{
   position_t left;
   positions_set right;
   Match(position_t left,positions_set right) :left(left), right(right){}
   friend std::ostream& operator<< (std::ostream &o, const Match &m)
   {
      o << "left: " << m.left;
      if (m.right.size()==0)
         o<<"\n";
      for (positions_set::const_iterator it=m.right.begin();it!=m.right.end();++it){
         o  << "\tright: " << *it << std::endl;
      }
      return o;
   }
};

void do_hash(const std::string src, hashes_vector& dest, bloom_bitset& bloom,const uint32_t window_size){
   const char* text = src.c_str();
   const position_t length = src.length()-window_size+1;
   for (position_t i=0;i<length;i++){
      dest[i]=hashmurmur(text+i,window_size+1); 
      // MurmurHash3_x64_128(text+i,window_size+1,8,&dest[i]);
      uint32_t bloom_hash = ((dest[i]>>BLOOM_BITS)^(dest[i]&BLOOM_MASK));
      bloom.set((dest[i]>>BLOOM_BITS)^(dest[i]&BLOOM_MASK));
   }
}

void dump(const std::deque<Match>& matches,const std::deque<Result>& results, const std::string& text, const std::string& other){
   std::cout << "Matches" << std::endl;
   for (uint32_t i=0;i<matches.size();i++){
      std::cout << matches[i];
   }
   std::cout << "Results" <<std::endl;
   for (uint32_t i=0;i<results.size();i++){
      std::cout << results[i] << "\"" << text.substr(results[i].left,results[i].length) << "\"\t\"" << other.substr(results[i].right,results[i].length) << "\"" << std::endl;
   }
}

void process_matches(std::deque<Match>& matches, std::deque<Result>& results,uint32_t window_size){
   while(matches.size()>0){
      Match* first = &matches.front();
      if (first->right.size()==0){
         matches.pop_front();
         continue; //Skip loop and progress forwards!
      }
      uint32_t first_right=*first->right.begin();
      first->right.erase(first->right.begin());
      uint32_t counter=1;
      while(counter<matches.size()){
         Match* next = &matches[counter];
         if ((next->left-first->left)==counter){
            positions_set::iterator next_right=next->right.find(first_right+counter);
            if (next_right!=next->right.end()){
               next->right.erase(*next_right);     
               counter++;
            }
            else{
               break;
            }
         }
         else{
            break;
         }
      }
      results.push_back(Result(first->left,first_right,counter+window_size));
   }
   sort(results.begin(),results.end(),result_sorter);
}

// Pass two strings to be compared and a integer for the windows_size
static int superfastmatch_match(lua_State *L)
{   
   //Init data structures
   double init_start = time();
   const std::string a = std::string(luaL_checkstring(L, 1));
   const std::string b = std::string(luaL_checkstring(L, 2));   
   uint32_t window_size = luaL_checkint(L, 3);
   hashes_vector a_hashes(a.length()-window_size+1,0);
   hashes_vector b_hashes(b.length()-window_size+1,0);
   bloom_bitset& a_bloom = *(new bloom_bitset());
   bloom_bitset& b_bloom = *(new bloom_bitset());
   double init_end = time();
   
   //Initialise hashes   
   double hash_start = time();

   // std::bitset<(1L<<30)-1>& test = *(new std::bitset<(1L<<30)-1>());
   do_hash(a,a_hashes,a_bloom,window_size);
   do_hash(b,b_hashes,b_bloom,window_size);
   double hash_end = time();

   double bloom_start = time();
   a_bloom &= b_bloom;
   double bloom_end = time();


   
   //Find a hashes set
   double a_hashes_start = time();
   hashes_set a_hashes_set;
   for (hashes_vector::iterator it=a_hashes.begin();it<a_hashes.end();++it){
      if (a_bloom[(*it>>BLOOM_BITS)^(*it&BLOOM_MASK)]){
         // std::cout << a_hashes_set.load_factor() << ":" << a_hashes_set.bucket_count() << std::endl;
         a_hashes_set.insert(*it);
      }
   }
   double a_hashes_end = time();  
   
   //Find b hashes set
   double b_hashes_start = time();
   matches_map b_matches;
   hashes_set::iterator a_hashes_set_end=a_hashes_set.end();
   for (position_t i=0;i<b_hashes.size();i++){
      if (a_bloom[(b_hashes[i]>>BLOOM_BITS)^(b_hashes[i]&BLOOM_MASK)] && a_hashes_set.find(b_hashes[i])!=a_hashes_set_end){
         b_matches[b_hashes[i]].insert(i);
      }
   }
   double b_hashes_end = time();   
   
   //Build matches
   double build_start = time();
   std::deque<Match> matches;
   matches_map::iterator b_matches_end=b_matches.end();
   for (uint32_t i=0;i<a_hashes.size();i++){
         matches_map::iterator b_match=b_matches.find(a_hashes[i]);
         if (b_match!=b_matches_end){
            matches.push_back(Match(i,b_match->second));
         }  
   }
   double build_end = time();
   
   //Process matches and build results
   double process_start = time();
   std::deque<Result> results;
   process_matches(matches,results,window_size);
   double process_end = time();

   std::cout << "a_hashes: " << a_hashes.size() << " a_bloom: " << a_bloom.count() << " b_hashes: " << b_hashes.size() << " b_bloom: " << b_bloom.count() << std::endl; 
   std::cout << "a_hashes_set: " << a_hashes_set.size() << ":" << a_hashes_set.bucket_count()  << " b_matches: " << b_matches.size() << ":" << b_matches.bucket_count() << " matches: " << matches.size() << " results: " << results.size() << std::endl;
   std::cout << "init: " << init_end-init_start << " hash: " << hash_end-hash_start << " bloom: "<< bloom_end-bloom_start << " a_hashes: " << a_hashes_end-a_hashes_start;
   std::cout << " b_hashes: " <<  b_hashes_end-b_hashes_start << " build: "<< build_end-build_start << " process: " << process_end-process_start << std::endl;   
   // dump(matches,results,a,b);
   
   return 1;
}

static const struct luaL_Reg superfastmatch_funcs[] = {
  { "match",	superfastmatch_match },
  { NULL, NULL }
};

extern "C"{ 
   LUALIB_API int luaopen_superfastmatch(lua_State *L)
   {
     luaL_register(L, "superfastmatch", superfastmatch_funcs); 
     return 1;
   }
}