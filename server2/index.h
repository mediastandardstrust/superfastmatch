#ifndef _SFMINDEX_H                       // duplication check
#define _SFMINDEX_H

#include <ktutil.h>
#include <kcutil.h>
#include <kcmap.h>
#include <kcdbext.h>
#include <kcpolydb.h>
#include <iostream>
#include <common.h>
#include <iterator>
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
		vector<uint32_t> existing_; // Stackoverflow?
		vector<uint32_t> merged_; // Stackoverflow?
		vector<uint32_t>::iterator existing_end_;
		vector<uint32_t>::iterator merged_end_;
	
 	public:
		uint32_t in_length;
		uint32_t out_length;
		uint32_t max_length;
		char* in;
		char* out;

		IndexLine(const uint32_t max_line_length):
		max_length(max_line_length*10),in(new char[max_line_length*10]),out(new char[max_line_length*10]){
			existing_.resize(max_line_length);
			merged_.resize(max_line_length);
		}
		
		~IndexLine(){
			delete[] in;
			delete[] out;
		}

	private:
		void read_existing(){
			uint32_t offset=0;
			uint64_t item;
			existing_end_=existing_.begin();
			while (offset<in_length){
				offset+=kyotocabinet::readvarnum(in+offset,in_length-offset,&item);
				*existing_end_=item;
				existing_end_++;
			};
		}
		
		bool write_merged(){
			out_length=0;
			for(vector<uint32_t>::iterator it=merged_.begin();it!=merged_end_;++it){
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
			vector<uint32_t>::const_iterator new_it=new_docs.begin();
			vector<uint32_t>::const_iterator new_ite=new_docs.end();
			vector<uint32_t>::iterator existing_it=existing_.begin();
			vector<uint32_t>::iterator merged_marker;
			merged_end_=merged_.begin();
			
			while(true){
				//If both exhausted
				if ((new_it==new_ite) && (existing_it==existing_end_)){
					// cout << "both exhausted" << endl;
					break;
				}
				
				//If new_docs is exhausted
				if (new_it==new_ite){
					// cout << "new docs exhausted"<<endl;
					merged_end_=copy(existing_it,existing_end_,merged_end_);
					break;
				}
				//If existing_docs is exhausted
				if (existing_it==existing_end_){
					// cout << "existing docs exhausted"<<endl;
					merged_end_=copy(new_it,new_ite,merged_end_);
					break;
				}
				//If doctypes are different merge lower doctype else merge contents
				uint32_t new_length=(*(new_it+1)+2);
				uint32_t existing_length=(*(existing_it+1)+2);
				// cout << "New Doctype: " << *new_it << " Existing Doctype: " << *existing_it <<endl;
				if (*new_it<*existing_it){
					// cout << "new is before existing"<<endl;
					merged_end_=copy(new_it,new_it+new_length,merged_end_);
					new_it+=new_length;
				}else if(*new_it>*existing_it){
					// cout << "existing is before new"<<endl;
					merged_end_=copy(existing_it,existing_it+existing_length,merged_end_);
					existing_it+=existing_length;
				}else{
					// cout << "merging"<<endl;
					*merged_end_=*new_it;
					merged_end_++;
					*merged_end_=0; //temporary length holder
					merged_marker=merged_end_;
					merged_end_++;
					merged_end_=set_union(new_it+2,new_it+new_length,existing_it+2,existing_it+existing_length,merged_end_);
					
					// cout << "Start: " << merged_docs_start << " End: " << merged_docs_end << endl;
					*merged_marker=distance(merged_marker,merged_end_)-1;
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
			// merged_docs_.resize(0);
			// merged_docs_.clear();
			// vector<uint32_t>::const_iterator new_it=new_docs.begin();
			// vector<uint32_t>::const_iterator new_ite=new_docs.end();
			// vector<uint32_t>::const_iterator existing_it=existing_docs_.begin();
			// vector<uint32_t>::const_iterator existing_ite=existing_docs_.end();
			// vector<uint32_t>::iterator merged_docs_length;
			// 
			// while(true){
			// 	//If both exhausted
			// 	if ((new_it==new_ite) && (existing_it==existing_ite)){
			// 		// cout << "both exhausted" << endl;
			// 		break;
			// 	}
			// 	
			// 	//If new_docs is exhausted
			// 	if (new_it==new_ite){
			// 		// cout << "new docs exhausted"<<endl;
			// 		copy(existing_it,existing_ite,back_inserter(merged_docs_));
			// 		break;
			// 	}
			// 	//If existing_docs is exhausted
			// 	if (existing_it==existing_ite){
			// 		// cout << "existing docs exhausted"<<endl;
			// 		break;
			// 	}
			// 	//If doctypes are different merge lower doctype else merge contents
			// 	uint32_t new_length=(*(new_it+1)+2);
			// 	uint32_t existing_length=(*(existing_it+1)+2);
			// 	// cout << "New Doctype: " << *new_it << " Existing Doctype: " << *existing_it <<endl;
			// 	if (*new_it<*existing_it){
			// 		// cout << "new is before existing"<<endl;
			// 		// copy(new_it,new_it+new_length,back_inserter(merged_docs_));
			// 		new_it+=new_length;
			// 	}else if(*new_it>*existing_it){
			// 		// cout << "existing is before new"<<endl;
			// 		copy(existing_it,existing_it+existing_length,back_inserter(merged_docs_));
			// 		existing_it+=existing_length;
			// 	}else{
			// 		//TODO consider deque instead
			// 		// cout << "removing"<<endl;
			// 		merged_docs_.push_back(*new_it);
			// 		merged_docs_.push_back(0); //temporary length holder
			// 		merged_docs_length = merged_docs_.end()-1;
			// 		uint32_t before_length=merged_docs_.size();
			// 		set_difference(existing_it+2,existing_it+existing_length,new_it+2,new_it+new_length,back_inserter(merged_docs_));
			// 		// set_union(new_it+2,new_it+new_length,existing_it+2,existing_it+existing_length,back_inserter(merged_docs_));
			// 		// cout << "Start: " << merged_docs_start << " End: " << merged_docs_end << endl;
			// 		*merged_docs_length=merged_docs_.size()-before_length;
			// 		new_it+=new_length;
			// 		existing_it+=existing_length;
			// 	}
			// 	
			// }
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
		IndexLine line_;
		
	public:
		Index(const Registry& registry):
		registry_(registry),line_(registry_.max_line_length){}
		
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
		
		void merge(kc::TinyHashMap& hashes,bool remove){
			if (remove){
				cout << "Removing " << hashes.count() << " hashes"<<endl;
			}else{
				cout << "Merging " << hashes.count() << " hashes"<<endl;
			}
			vector<uint32_t> new_docs;
			vector<uint32_t> docs;
			kc::TinyHashMap::Sorter sorter(&hashes);
			const char* hash, *doc_pairs;
			size_t hash_size, doc_pair_size;
			while ((hash=sorter.get(&hash_size, &doc_pairs, &doc_pair_size)) != NULL) {
				uint64_t item;
				uint32_t offset=0;
				while (offset<doc_pair_size){
					offset+=kyotocabinet::readvarnum(doc_pairs+offset,doc_pair_size-offset,&item);
					docs.push_back(item);
				}; 
				//Fold the doctype and docids into merge format
				uint32_t doc_type_cursor=1;
				uint32_t doc_type_count=1;
				for (uint32_t i=0;i<docs.size();i+=2){
					if(i==0){
						new_docs.push_back(docs[0]);
					}else if (docs[i]==new_docs[doc_type_cursor-1]){
						doc_type_count++;
					}else{
						new_docs.push_back(doc_type_count);
						new_docs.push_back(docs[i]);
						doc_type_count=1;
						doc_type_cursor++;
					}
				}
				new_docs.push_back(doc_type_count);
				for (uint32_t i=1;i<docs.size();i+=2){
					new_docs.push_back(docs[i]);
				}
				int32_t size=registry_.indexDB->get(hash,hash_size,line_.in,line_.max_length);
				line_.in_length=(size!=-1)?size:0;
				if (remove && line_.remove(new_docs)){
					registry_.indexDB->set(hash,hash_size,line_.out,line_.out_length);									
				}
				else if (line_.merge(new_docs)){
					registry_.indexDB->set(hash,hash_size,line_.out,line_.out_length);				
				}
				new_docs.clear();
				docs.clear();
				sorter.step();
			}
			hashes.clear();
		}
		
	public:
		bool batch(deque<Document*>& docs,bool remove){
			kc::TinyHashMap hashes(registry_.max_hash_count);
			std::sort(docs.begin(),docs.end(),compareDocuments());
			char hash[sizeof(hash_t)];
			char doc_pair[20];
			while (!docs.empty()){
				Document* doc = docs.front();
				cout <<"Indexing: " << *doc << endl;
				uint32_t doc_pair_length=0;
				doc_pair_length+=kc::writevarnum(doc_pair,doc->doctype());
				doc_pair_length+=kc::writevarnum(doc_pair+doc_pair_length,doc->docid());
				for (Document::hashes_vector::const_iterator it=doc->unique_sorted_hashes().begin(),ite=doc->unique_sorted_hashes().end();it!=ite;++it){
					hash_t hash_int = kc::hton32(*it);
					memcpy(hash,&hash_int,sizeof(hash_t));
					hashes.append(hash,sizeof(hash_t),doc_pair,doc_pair_length);
				}
				if (hashes.count()>registry_.max_hash_count){
					cout << "Hash count limit reached: " << hashes.count() << " > " << registry_.max_hash_count << endl;
					merge(hashes,remove);
				}
				doc->clear();
				docs.pop_front();
			}
			merge(hashes,remove);
			return true;
		}
	};
}
#endif