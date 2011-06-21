#ifndef _SFMINDEX_H                       // duplication check
#define _SFMINDEX_H

#include <ktutil.h>
#include <kcutil.h>
#include <kcdbext.h>
#include <kcpolydb.h>
#include <iostream>
#include <common.h>
#include <algorithm>
#include <tr1/unordered_map>
#include <cstring>
#include <registry.h>
#include <job.h>
#include <document.h>

namespace superfastmatch
{
	class Document;
	
	// Optimised to be reusable for the whole merge and prevent any new-ing or delete-ing
	// Stopped using unordered_map as it was heavier than merging vectors
	// Horrible but fast!
	class IndexLine{
	private:
		vector<uint32_t> existing_docs_; // Stackoverflow?
		vector<uint32_t> merged_docs_; // Stackoverflow?
 	public:
		uint32_t in_length;
		uint32_t out_length;
		uint32_t max_length;
		char* in;
		char* out;

		IndexLine(const uint32_t max_line_length):
		max_length(max_line_length*10),in(new char[max_line_length*10]),out(new char[max_line_length*10]){
			existing_docs_.reserve(max_line_length);
			merged_docs_.reserve(max_line_length);
		}
		
		~IndexLine(){
			delete[] in;
			delete[] out;
		}

	private:
		void read_existing(){
			uint32_t offset=0;
			uint64_t item;
			existing_docs_.resize(0); //slow?
			while (offset<in_length){
				offset+=kyotocabinet::readvarnum(in+offset,in_length-offset,&item);
				existing_docs_.push_back(item);
			};
		}
		
		bool write_merged(){
			out_length=0;
			for(vector<uint32_t>::iterator it=merged_docs_.begin(),ite=merged_docs_.end();it!=ite;++it){
				out_length+=kyotocabinet::writevarnum(out+out_length,*it);
			}
			if (in_length!=out_length){
				return true;
			}
			return (strncmp(in,out,in_length)!=0);
		}

	public:
		//Returns true if merge results in a change
		bool merge(const vector<uint32_t>& new_docs){
			read_existing();
			// cout << "Start In Length: " << in_length << " Out Length: " << out_length <<" Existing size: " << existing_docs_.size() << " New size: " << new_docs.size() << " Merged size: " << merged_docs_.size() <<endl;
			merged_docs_.resize(0);
			vector<uint32_t>::const_iterator new_it=new_docs.begin();
			vector<uint32_t>::const_iterator new_ite=new_docs.end();
			vector<uint32_t>::const_iterator existing_it=existing_docs_.begin();
			vector<uint32_t>::const_iterator existing_ite=existing_docs_.end();
			vector<uint32_t>::iterator merged_docs_length;
			
			while(true){
				//If both exhausted
				if ((new_it==new_ite) && (existing_it==existing_ite)){
					// cout << "both exhausted" << endl;
					break;
				}
				
				//If new_docs is exhausted
				if (new_it==new_ite){
					// cout << "new docs exhausted"<<endl;
					copy(existing_it,existing_ite,back_inserter(merged_docs_));
					break;
				}
				//If existing_docs is exhausted
				if (existing_it==existing_ite){
					// cout << "existing docs exhausted"<<endl;
					copy(new_it,new_ite,back_inserter(merged_docs_));
					break;
				}
				//If doctypes are different merge lower doctype else merge contents
				uint32_t new_length=(*(new_it+1)+2);
				uint32_t existing_length=(*(existing_it+1)+2);
				// cout << "New Doctype: " << *new_it << " Existing Doctype: " << *existing_it <<endl;
				if (*new_it<*existing_it){
					// cout << "new is before existing"<<endl;
					copy(new_it,new_it+new_length,back_inserter(merged_docs_));
					new_it+=new_length;
				}else if(*new_it>*existing_it){
					// cout << "existing is before new"<<endl;
					copy(existing_it,existing_it+existing_length,back_inserter(merged_docs_));
					existing_it+=existing_length;
				}else{
					// cout << "merging"<<endl;
					merged_docs_.push_back(*new_it);
					merged_docs_.push_back(0); //temporary length holder
					merged_docs_length = merged_docs_.end()-1;
					uint32_t before_length=merged_docs_.size();
					set_union(new_it+2,new_it+new_length,existing_it+2,existing_it+existing_length,back_inserter(merged_docs_));
					// cout << "Start: " << merged_docs_start << " End: " << merged_docs_end << endl;
					*merged_docs_length=merged_docs_.size()-before_length;
					new_it+=new_length;
					existing_it+=existing_length;
				}
				
			}
			// cout << "End In Length: " << in_length << " Out Length: " << out_length <<" Existing size: " << existing_docs_.size() << " New size: " << new_docs.size() << " 	Merged size: " << merged_docs_.size() <<endl;
			return write_merged();
		}
		
		bool remove(const vector<uint32_t>& new_docs){
			read_existing();
			// cout << "Start In Length: " << in_length << " Out Length: " << out_length <<" Existing size: " << existing_docs_.size() << " New size: " << new_docs.size() << " Merged size: " << merged_docs_.size() <<endl;
			merged_docs_.resize(0);
			vector<uint32_t>::const_iterator new_it=new_docs.begin();
			vector<uint32_t>::const_iterator new_ite=new_docs.end();
			vector<uint32_t>::const_iterator existing_it=existing_docs_.begin();
			vector<uint32_t>::const_iterator existing_ite=existing_docs_.end();
			vector<uint32_t>::iterator merged_docs_length;
			
			while(true){
				//If both exhausted
				if ((new_it==new_ite) && (existing_it==existing_ite)){
					// cout << "both exhausted" << endl;
					break;
				}
				
				//If new_docs is exhausted
				if (new_it==new_ite){
					// cout << "new docs exhausted"<<endl;
					copy(existing_it,existing_ite,back_inserter(merged_docs_));
					break;
				}
				//If existing_docs is exhausted
				if (existing_it==existing_ite){
					// cout << "existing docs exhausted"<<endl;
					break;
				}
				//If doctypes are different merge lower doctype else merge contents
				uint32_t new_length=(*(new_it+1)+2);
				uint32_t existing_length=(*(existing_it+1)+2);
				// cout << "New Doctype: " << *new_it << " Existing Doctype: " << *existing_it <<endl;
				if (*new_it<*existing_it){
					// cout << "new is before existing"<<endl;
					// copy(new_it,new_it+new_length,back_inserter(merged_docs_));
					new_it+=new_length;
				}else if(*new_it>*existing_it){
					// cout << "existing is before new"<<endl;
					copy(existing_it,existing_it+existing_length,back_inserter(merged_docs_));
					existing_it+=existing_length;
				}else{
					//TODO consider deque instead
					// cout << "removing"<<endl;
					merged_docs_.push_back(*new_it);
					merged_docs_.push_back(0); //temporary length holder
					merged_docs_length = merged_docs_.end()-1;
					uint32_t before_length=merged_docs_.size();
					set_difference(existing_it+2,existing_it+existing_length,new_it+2,new_it+new_length,back_inserter(merged_docs_));
					// set_union(new_it+2,new_it+new_length,existing_it+2,existing_it+existing_length,back_inserter(merged_docs_));
					// cout << "Start: " << merged_docs_start << " End: " << merged_docs_end << endl;
					*merged_docs_length=merged_docs_.size()-before_length;
					new_it+=new_length;
					existing_it+=existing_length;
				}
				
			}
			// cout << "End In Length: " << in_length << " Out Length: " << out_length <<" Existing size: " << existing_docs_.size() << " New size: " << new_docs.size() << " 	Merged size: " << merged_docs_.size() <<endl;
			return write_merged();
		}
		
	};
	
	class Index
	{
	private:
		typedef vector<uint32_t> docids_vector;
		typedef unordered_map<uint32_t,docids_vector> hashes_map;
		
		const Registry& registry_;
		
	public:
		Index(const Registry& registry):
		registry_(registry){}
		
		~Index(){
			// printf("Destroyed Indexer (%p)\n", this);
		}
		
	private:
		struct compareDocuments {
		  	bool operator ()(Document* lhs, Document* rhs){
			 	if (lhs->doctype() == rhs->doctype()){
					return lhs->docid() < rhs->docid();
				} 
				return lhs->doctype() < rhs->doctype();
			}
		};
		
		void merge(hashes_map& hashes,bool remove){
			if (remove){
				cout << "Removing " << hashes.size() << " hashes"<<endl;
			}else{
				cout << "Merging " << hashes.size() << " hashes"<<endl;
			}
			char hash[sizeof(hash_t)];
			IndexLine line(registry_.max_line_length);
			vector<uint32_t> new_docs;
			vector<hash_t> sorted_hashes;
			sorted_hashes.reserve(hashes.size());
			for (hashes_map::const_iterator it=hashes.begin(),ite=hashes.end(); it!=ite; ++it) {
				if (it->second.size()>0){
			    	sorted_hashes.push_back(it->first);
				}
			}
			std::sort(sorted_hashes.begin(),sorted_hashes.end());
			for (vector<hash_t>::const_iterator it=sorted_hashes.begin(),ite=sorted_hashes.end(); it!=ite; ++it) {
				//Fold the doctype and docids into merge format
				uint32_t doc_type_cursor=1;
				uint32_t doc_type_count=1;
				for (uint32_t i=0;i<hashes[*it].size();i+=2){
					if(i==0){
						new_docs.push_back(hashes[*it][0]);
					}else if (hashes[*it][i]==new_docs[doc_type_cursor-1]){
						doc_type_count++;
					}else{
						new_docs.push_back(doc_type_count);
						new_docs.push_back(hashes[*it][i]);
						doc_type_count=1;
						doc_type_cursor++;
					}
				}
				new_docs.push_back(doc_type_count);
				for (uint32_t i=1;i<hashes[*it].size();i+=2){
					new_docs.push_back(hashes[*it][i]);
				}
				uint32_t hash_int = kc::hton32(*it);
				memcpy(hash,&hash_int,sizeof(hash_t));
				int32_t size=registry_.indexDB->get(hash,sizeof(hash_t),line.in,line.max_length);
				line.in_length=(size!=-1)?size:0;
				if (remove && line.remove(new_docs)){
					registry_.indexDB->set(hash,sizeof(hash_t),line.out,line.out_length);									
				}
				else if (line.merge(new_docs)){
					registry_.indexDB->set(hash,sizeof(hash_t),line.out,line.out_length);				
				}
				new_docs.clear();
				// http://www.gotw.ca/gotw/054.htm
				docids_vector().swap(hashes[*it]);
			}
		}
		
	public:
		bool batch(deque<Document*>& docs,bool remove){
			hashes_map hashes;
			hashes.rehash(registry_.max_hash_count/hashes.max_load_factor());
			std::sort(docs.begin(),docs.end(),compareDocuments());
			while (!docs.empty()){
				Document* doc = docs.front();
				cout <<"Indexing: " << *doc << endl;
				for (Document::hashes_vector::const_iterator it=doc->unique_sorted_hashes().begin(),ite=doc->unique_sorted_hashes().end();it!=ite;++it){
					hashes[*it].push_back(doc->doctype());
					hashes[*it].push_back(doc->docid());
				}
				if (hashes.size()>registry_.max_hash_count){
					cout << "Hash count limit reached: " << hashes.size() << " > " << registry_.max_hash_count << endl;
					merge(hashes,remove);
				}
				docs.pop_front();
			}
			merge(hashes,remove);
			return true;
		}
	};
}
#endif