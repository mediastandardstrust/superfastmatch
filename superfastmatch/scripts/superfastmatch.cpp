#define LUA_SUPERFASTMATCH_VERSION	"0.4"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include <utility>
#include <iostream>
#include <deque>
#include <cstring>
#include <bitset>
#include <vector>
#include <algorithm>
#include <iterator>
#include <tr1/unordered_set>
#include <tr1/unordered_map>


#include "MurmurHash3.h"
#include "kcutil.h"
#include "kcmap.h"

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

void validate(const std::deque<Result>& results, const char* a, const char* b){
   std::string text = std::string(a);
   std::string other = std::string(b);
   for (uint32_t i=0;i<results.size();i++){
      if (text.substr(results[i].left,results[i].length)!=other.substr(results[i].right,results[i].length)){
         std::cout << results[i] << "\"" << text.substr(results[i].left,results[i].length) << "\"\t\"" << other.substr(results[i].right,results[i].length) << "\"" << std::endl;
      }
   }
}

void dump(const std::deque<Match>& matches,const std::deque<Result>& results, const char* a, const char* b){
   std::string text = std::string(a);
   std::string other = std::string(b);
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

inline hash_t x31_hash(const char *s,position_t length)
{
	hash_t h = *s;
   for (position_t i=0;i<length;i++){
	   h = (h << 5) - h + *(s+i);
	}
	return h;
}

inline hash_t larsen_hash(const char *s, position_t length) {
	hash_t hash = 0;
	for(position_t i = 0; i < length; ++i)
		hash = 101 * hash + s[i];
	return hash ^ (hash >> 16);
}

// Pass two strings to be compared and a integer for the windows_size
static int superfastmatch_match(lua_State *L)
{   
   //Init data structures
   double init_start = time();
   const char* a = luaL_checkstring(L, 1);
   const char* b = luaL_checkstring(L, 2);   
   const position_t window_size = luaL_checkint(L, 3)-1;
   const position_t a_length = strlen(a)-window_size;
   const position_t b_length = strlen(b)-window_size;   
   hashes_vector a_hashes(a_length,0);
   bloom_bitset a_bloom;
   bloom_bitset b_bloom;
   hash_t hash;
   for (position_t i=0;i<a_length;i++){
      hash=hashmurmur(a+i,window_size+1); 
      a_hashes[i]=hash;
      a_bloom.set(hash&BLOOM_MASK);
   }
   std::vector<position_t> b_picks;
   hashes_vector b_hashes(b_length,0);
   for (position_t i=0;i<b_length;i++){
      hash=hashmurmur(b+i,window_size+1);
      if (a_bloom.test(hash&BLOOM_MASK)){
         b_picks.push_back(i); 
         b_hashes[i]=hash; 
         b_bloom.set(hash&BLOOM_MASK);
      }
   }
   
   hashes_set a_hashes_set;
   for (hashes_vector::iterator it=a_hashes.begin();it<a_hashes.end();++it){
      if (b_bloom.test(*it&BLOOM_MASK)){
         a_hashes_set.insert(*it);
      }
   }
   
   //Find b hashes set
   matches_map b_matches;
   hashes_set::iterator a_hashes_set_end=a_hashes_set.end();
   for (position_t i=0;i<b_picks.size();i++){
      hash=b_hashes[b_picks[i]];
      if (a_hashes_set.find(hash)!=a_hashes_set_end){
         b_matches[hash].insert(b_picks[i]);
      }
   }

   //Build matches
   double build_start = time();
   std::deque<Match> matches;
   matches_map::iterator b_matches_end=b_matches.end();
   for (uint32_t i=0;i<a_hashes.size();i++){
         matches_map::iterator b_match=b_matches.find(a_hashes[i]);
         if (b_match!=b_matches_end){
            //TODO Make this a vector
            positions_set checked_matches(b_match->second);
            for (positions_set::iterator it=checked_matches.begin();it!=checked_matches.end();++it){
               if (strncmp(a+i,b+*it,window_size)){
                  checked_matches.erase(it);
               }
            }
            matches.push_back(Match(i,checked_matches));
         }  
   }
   
   //Process matches and build results
   double process_start = time();
   std::deque<Result> results;
      // dump(matches,results,a,b);      
   process_matches(matches,results,window_size);
   double process_end = time();

   std::cout << "a_hashes: " << a_hashes.size() << " a_bloom: " << a_bloom.count() << " b_hashes: " << b_hashes.size() << " b_bloom: " << b_bloom.count() << std::endl; 
   std::cout << "a_hashes_set: " << a_hashes_set.size() << ":" << a_hashes_set.bucket_count()  << " b_matches: " << b_matches.size() << ":" << b_matches.bucket_count() << " matches: " << matches.size() << " results: " << results.size() << std::endl;
   // std::cout << "init: " << init_end-init_start << " hash: " << hash_end-hash_start << " bloom: "<< bloom_end-bloom_start << " a_hashes: " << a_hashes_end-a_hashes_start;
   // std::cout << " b_hashes: " <<  b_hashes_end-b_hashes_start << " build: "<< build_end-build_start << " process: " << process_end-process_start << std::endl;   
   // validate(results,a,b);      
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