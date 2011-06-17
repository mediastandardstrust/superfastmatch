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

typedef vector<uint32_t> docids_vector;
typedef std::tr1::unordered_map<uint32_t,docids_vector> docs_map;

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
		
		bool remove(const vector<uint32_t>& old_docs){
			read_existing();
			
			return write_merged();
		}
		
	};
	
	class IndexVisitor: public kyotocabinet::DB::Visitor 
	{
	private:
		const Registry& registry_;	
		const Job& job_;
		IndexLine line_;
		unordered_map<string,string> visited_;
		vector<uint32_t> new_docs_;
		uint64_t hash_count_;
		unordered_map<hash_t,vector<uint64_t> > new_docs_map_;			
		
		void dump(){
			cout << "Hash count limit reached" << endl;
			vector<hash_t> sorted_hashes;
			sorted_hashes.reserve(new_docs_map_.size());
			for (unordered_map<hash_t,vector<uint64_t> >::const_iterator it=new_docs_map_.begin(),ite=new_docs_map_.end(); it!=ite; ++it) {
			    sorted_hashes.push_back(it->first);
			}
			std::sort(sorted_hashes.begin(),sorted_hashes.end());
			for (vector<hash_t>::const_iterator it=sorted_hashes.begin(),ite=sorted_hashes.end(); it!=ite; ++it) {
			
			}
			new_docs_map_.clear();
			hash_count_=0;
		}
		
	public:
		IndexVisitor(const Registry& registry,Job& job):
		registry_(registry),job_(job),line_(registry_.max_line_length),hash_count_(0)
		{}
		
		~IndexVisitor(){
			dump();
		}
		
	    const char* visit_full(const char* kbuf, size_t ksiz,const char* vbuf, size_t vsiz, size_t *sp) {
			string doc_key = string(kbuf, ksiz);
			visited_[doc_key]=string(vbuf, vsiz);
			Document document(doc_key,registry_);
			cout << "Batching: " << document <<endl;
			for (Document::hashes_vector::const_iterator it=document.unique_sorted_hashes().begin(),ite=document.unique_sorted_hashes().end();it!=ite;++it){
				new_docs_map_[*it].push_back(document.doctype());
				new_docs_map_[*it].push_back(document.docid());
				hash_count_++;
			}
			if (hash_count_>registry_.max_hash_count){
				dump();	
			}
			return NOP;
	    }

		void visit_after(){
		}
	
	};
	
	class Index
	{
	private:
		const Registry& registry_;
	public:
		Index(const Registry& registry):
		registry_(registry){}
		
		~Index(){
			// printf("Destroyed Indexer (%p)\n", this);
		}

		bool create(Document& document){
			IndexLine line(registry_.max_line_length);
			char hash[sizeof(hash_t)];
			vector<uint32_t> new_docs;
			new_docs.push_back(document.doctype());
			new_docs.push_back(1);
			new_docs.push_back(document.docid());
			for (Document::hashes_vector::const_iterator it=document.unique_sorted_hashes().begin(),ite=document.unique_sorted_hashes().end();it!=ite;++it){
				uint32_t hash_int = kc::hton32(*it);
				memcpy(&hash,&hash_int,sizeof(hash_t));
				int32_t size=registry_.indexDB->get(hash,sizeof(hash_t),line.in,line.max_length);
				line.in_length=(size!=-1)?size:0;
				if (line.merge(new_docs)){
					registry_.indexDB->set(hash,sizeof(hash_t),line.out,line.out_length);				
				}
			}
			return true;
		}

		bool batch(Job& job){
			if(!job.start()){
				return false;				
			}
			vector<pair<string,string> > doc_tasks;
			string doc_key;
			string timestamp;
			kyotocabinet::PolyDB::Cursor* cur = registry_.queueDB->cursor();
			cur->jump();
			while (cur->get(&doc_key,&timestamp,true)){
				doc_tasks.push_back(pair<string,string>(doc_key,timestamp));
				Document document(doc_key,registry_);
				cout << document <<endl;
			}
			bool success;
			// IndexVisitor* visitor = new IndexVisitor(registry_,job);
			// bool success = registry_.hash_queueDB->iterate(visitor,false);
			// delete visitor;
			return success && job.finish();
		}
	};
}
#endif