#ifndef _SFMMATCHER_H                       // duplication check
#define _SFMMATCHER_H

#include "common.h"
#include "document.h"
#include <iostream>
#include <utility>
#include <string>
#include <deque>
#include <tr1/unordered_set>
#include <tr1/unordered_map>

namespace Superfastmatch
{
	struct Match;
	struct Result;
	
	typedef std::tr1::unordered_set<uint32_t> hashes_set;
	typedef std::tr1::unordered_set<uint32_t> positions_set;
	typedef std::tr1::unordered_map<uint32_t,positions_set> matches_map;	
	typedef std::pair<uint32_t,positions_set> match_pair;
	typedef std::deque<Match> matches_deque;
	typedef std::deque<Result> results_deque;
	
	struct Match{
	   uint32_t left;
	   positions_set right;
	   Match(uint32_t left,positions_set right) :left(left), right(right){}
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
	
	struct Result{
	   uint32_t left;
	   uint32_t right;
	   uint32_t length;
	   Result(uint32_t left,uint32_t right,uint32_t length) : left(left), right(right), length(length){}
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
	
	
	class Matcher
	{
	public:
		//C++ interface		
		void match(Document* from_document, Document* to_document, uint32_t windowsize){
			Document::hashes_bloom bloom = from_document->bloom()&to_document->bloom();
			Document::hashes_vector from_hashes = from_document->hashes();
			Document::hashes_vector to_hashes   = to_document->hashes();
			uint32_t from_hashes_count = from_hashes.size();
			uint32_t to_hashes_count   = to_hashes.size();
			std::string from_text = from_document->text();
			std::string to_text = to_document->text();
			
			//Find from_document hashes set
		    hashes_set from_hashes_set;
			for (uint32_t i =0;i<from_hashes_count;i++){
		    	if (bloom.test(from_hashes[i]&0xFFFFFF)){
		         	from_hashes_set.insert(from_hashes[i]);
		      	}
		    }
			
			//Find to_document hashes map
		   	matches_map to_matches;
			hashes_set::iterator from_hashes_set_end=from_hashes_set.end();
			uint32_t hash;
			for (uint32_t i=0;i<to_hashes_count;i++){
				hash=to_hashes[i];
				if (bloom.test(hash&0xFFFFFF)){
					if (from_hashes_set.find(hash)!=from_hashes_set_end){
						to_matches[hash].insert(i);		
					}
				}
		   }
		
		   //Build matches
		   matches_deque matches;
		   matches_map::iterator to_matches_end=to_matches.end();
		   for (uint32_t i=0;i<from_hashes_count;i++){
		         matches_map::iterator to_match=to_matches.find(from_hashes[i]);
		         if (to_match!=to_matches_end){
		            positions_set checked_matches(to_match->second);
		            for (positions_set::iterator it=checked_matches.begin();it!=checked_matches.end();++it){
						if (from_text.compare(i,windowsize,to_text,*it,windowsize)){
							checked_matches.erase(it);
					    }
					}
					matches.push_back(Match(i,checked_matches));
			     }  
			}
			
			//Process matches to find longest common strings
			results_deque results;
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
			      Match* next = &matches[counter];;
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
			   results.push_back(Result(first->left,first_right,counter+windowsize));
			}
			sort(results.begin(),results.end(),result_sorter);
			
			std::cout << "Matched: " << *from_document << " with: " << *to_document;
		    std::cout << " Bloom size: " << bloom.count()<< " From hashes: " << from_document->hashes().size() <<" To hashes: " << to_document->hashes().size();
			std::cout << " From hashes set: " << from_hashes_set.size() << " To hashes map: " << to_matches.size() << " Matches: " << matches.size();
			std::cout << " Results: " << results.size() << std::endl;
			// for (uint32_t i=0;i<results.size();i++){
			// 		      	std::cout << results[i] << "\"" << from_text.substr(results[i].left,results[i].length) << "\"\t\"" << to_text.substr(results[i].right,results[i].length) << "\"" << std::endl;
			// }
		}
		
		~Matcher(){
			printf("deleted Matcher (%p)\n", this);
		}
		
		//Lua Interface
		Matcher(lua_State *L)
		{
		};		
		
		int match(lua_State *L){
			Document* from_document = Lunar<Document>::check(L, 1);
			Document* to_document = Lunar<Document>::check(L, 2);
			match(from_document,to_document,from_document->windowsize());
			return 0;
		}		
		
		static const char className[];
		static Lunar<Matcher>::RegType methods[];

	};
	
	const char Matcher::className[] = "Matcher";
	
	Lunar<Matcher>::RegType Matcher::methods[] = {
	  LUNAR_DECLARE_METHOD(Matcher, match),
	  {0,0}
	};
}//namespace Superfastmatch

#endif                                   // duplication check