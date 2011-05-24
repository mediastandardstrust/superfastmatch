#define LUA_SUPERFASTMATCH_VERSION	"0.4"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include <iostream>
#include <deque>
#include <string>
#include <vector>
#include <algorithm>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <utility>
#include "kcutil.h"
using namespace std;
using namespace std::tr1;
using namespace kyotocabinet;

#ifdef _MSC_VER
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

struct Result{
   uint32_t left;
   uint32_t right;
   uint32_t length;
   Result(uint32_t left,uint32_t right,uint32_t length) : left(left), right(right), length(length){}
   friend std::ostream& operator<< (std::ostream &o, const Result &r)
   {
      return o << "left: " << r.left << " right: " << r.right << " length: " << r.length <<'\n';
   }
};

bool result_sorter(Result const& lhs,Result const& rhs ){
   if (lhs.length!=rhs.length)
      return lhs.length > rhs.length;
   if (lhs.left!=rhs.left)
      return lhs.left < rhs.left;
   return lhs.right < rhs.right;
}

typedef pair<uint32_t,uint32_t> match;

void do_hash(const string& src, vector<uint64_t>& dest,uint32_t window_size){
   for (uint32_t i=0;i<src.length()-window_size+1;i++){
      dest[i]=hashmurmur(src.substr(i,window_size).c_str(),window_size);
   }
}

void process_matches(deque<match>& matches, deque<Result>& results,uint32_t window_size){
   while(matches.size()>1){
      match first = matches.front();
      matches.pop_front();
      uint32_t counter = 0;
      uint32_t length = window_size;
      while(counter<matches.size()){
         uint32_t second_pos = counter;
         match second = matches[second_pos];
         uint32_t limit = length-window_size+1;
         uint32_t left = second.first-first.first;
         uint32_t right = second.second-first.second;
         if (left!=right and left<=limit){
            counter++;
         }
         else if (left>limit){
            break;
         }
         else{    //left==right
            matches.erase(matches.begin()+second_pos);
            length++;
         }
      }
      results.push_back(Result(first.first,first.second,length));
   }
   if (matches.size()==1)
      results.push_back(Result(matches.front().first,matches.front().second,window_size));
   sort(results.begin(),results.end(),result_sorter);
}

// Pass two strings to be compared and a integer for the windows_size
static int superfastmatch_match(lua_State *L)
{
   //Get parameters
   const string a = string(luaL_checkstring(L, 1));
   const string b = string(luaL_checkstring(L, 2));   
   uint32_t window_size = luaL_checkint(L, 3);
   
   //Initialise hashes   
   vector<uint64_t> a_hashes(a.length()-window_size+1,0);
   vector<uint64_t> b_hashes(b.length()-window_size+1,0);
   do_hash(a,a_hashes,window_size);
   do_hash(b,b_hashes,window_size);
   
   //Find common hashes
   unordered_set<uint64_t> a_hashes_set (a_hashes.begin(),a_hashes.end());
   unordered_set<uint64_t> common;
   unordered_map<uint64_t,deque<uint32_t> > b_matches; //TODO Try swapping for vector
   for (uint32_t i=0;i<b_hashes.size();i++){
      if (a_hashes_set.find(b_hashes[i])!=a_hashes_set.end()){
         b_matches[b_hashes[i]].push_back(i);
         common.insert(b_hashes[i]);
      }
   }
   
   //Build matches
   deque<match> matches;
   for (uint32_t i=0;i<a_hashes.size();i++){
      if (common.find(a_hashes[i])!=common.end()){
         for (uint32_t j=0;j<b_matches[a_hashes[i]].size();j++){
            matches.push_back(match(i,b_matches[a_hashes[i]][j]));  
         }
      }
   }
   
   //Process matches and build results
   deque<Result> results;
   process_matches(matches,results,window_size);


   cout << "intersection has " << common.size() << " elements.\n";
   cout << "b_matches has " << b_matches.size() << " elements.\n";
   
   // for(uint32_t i=0;i<results.size();i++){
   //    cout << results[i];
   //    cout << a.substr(results[i].left,results[i].length) << endl;
   // }

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