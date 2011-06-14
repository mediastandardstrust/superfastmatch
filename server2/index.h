#ifndef _SFMINDEX_H                       // duplication check
#define _SFMINDEX_H

#include <kttimeddb.h>
#include <ktutil.h>
#include <kcutil.h>
#include <kcdbext.h>
#include <kcpolydb.h>
#include <registry.h>
#include <job.h>
#include <document.h>
#include <iostream>
#include <common.h>
#include <algorithm>
#include <tr1/unordered_map>
#include <cstring>

typedef vector<uint32_t> docids_vector;
typedef std::tr1::unordered_map<uint32_t,docids_vector> docs_map;



namespace superfastmatch
{
	// Optimised to be reusable for the whole merge and prevent any new-ing or delete-ing
	// Stopped using unordered_map as it was heavier than merging vectors
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
			cout << "Start In Length: " << in_length << " Out Length: " << out_length <<" Existing size: " << existing_docs_.size() << " New size: " << new_docs.size() << " Merged size: " << merged_docs_.size() <<endl;
			merged_docs_.resize(0);
			vector<uint32_t>::const_iterator new_it=new_docs.begin();
			vector<uint32_t>::const_iterator new_ite=new_docs.end();
			vector<uint32_t>::const_iterator existing_it=existing_docs_.begin();
			vector<uint32_t>::const_iterator existing_ite=existing_docs_.end();
			vector<uint32_t>::iterator merged_docs_back;
			while(true){
				//If new_docs is exhausted
				if (new_it==new_ite){
					cout << "new docs exhausted"<<endl;
					copy(existing_it,existing_ite,back_inserter(merged_docs_));
					break;
				}
				//If existing_docs is exhausted
				if (existing_it==existing_ite){
					cout << "existing docs exhausted"<<endl;
					copy(new_it,new_ite,back_inserter(merged_docs_));
					break;
				}
				//If doctypes are different merge lower doctype else merge contents
				uint32_t new_length=(*(new_it+1)+2);
				uint32_t existing_length=(*(existing_it+1)+2);
				cout << "New Doctype: " << *new_it << " Existing Doctype: " << *existing_it <<endl;
				if (*new_it<*existing_it){
					cout << "new is before existing"<<endl;
					copy(new_it,new_it+new_length,back_inserter(merged_docs_));
					new_it+=new_length;
				}else if(*new_it>*existing_it){
					cout << "existing is before new"<<endl;
					copy(existing_it,existing_it+existing_length,back_inserter(merged_docs_));
					existing_it+=existing_length;
				}else{
					cout << "merging"<<endl;
					merged_docs_.push_back(*new_it);
					merged_docs_.push_back(0); //temporary length holder
					merged_docs_back = merged_docs_.end()-1;
					set_union(new_it+2,new_it+new_length,existing_it+2,existing_it+existing_length,back_inserter(merged_docs_));
					*merged_docs_back=merged_docs_.end()-merged_docs_back;
					new_it+=new_length;
					existing_it+=existing_length;
				}
				
			}
			cout << "End In Length: " << in_length << " Out Length: " << out_length <<" Existing size: " << existing_docs_.size() << " New size: " << new_docs.size() << " 	Merged size: " << merged_docs_.size() <<endl;
			return write_merged();
		}
		
		bool remove(const vector<uint32_t>& old_docs){
			read_existing();
			
			return write_merged();
		}
		
	};
	
	class Indexer : public kyotocabinet::MapReduce 
	{
	private:
		const Registry& registry_;
		Job& job_;
	public:
		explicit Indexer(const Registry& registry,Job& job):
		registry_(registry),job_(job)
		{}
	
	private:
		bool log(const char* name, const char* message) {
			return job_.log(name,message);
		} 	
	
	    bool map(const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) {
			// std::string job_key = std::string(kbuf,ksiz);
			// std::string doc_key;
			// char hash[sizeof(hash_t)];
			// char index_key[8];
			// if (job_.isJobItem(job_key,doc_key)){
			// 	Document document(doc_key,registry_);
			// 	Document::hashes_vector::const_iterator ite=document.hashes().end();
			// 	for (Document::hashes_vector::const_iterator it=document.hashes().begin();it!=ite;++it){
			// 		kc::writefixnum(hash,kc::hton32(*it),sizeof(hash_t));
			// 		kc::writefixnum(index_key,document.index_key(),8);
			// 		emit(hash, sizeof(hash_t), index_key, 8);
			// 	}		        
			// }
	      	return true;
	    }

	    bool reduce(const char* kbuf, size_t ksiz, ValueIterator* iter) {
			// string hash = string(kbuf,ksiz);
			// docs_vector new_docs;
			// const char* vbuf;
			// size_t vsiz;
			// uint32_t doc_type;
			// uint32_t doc_id;
			// while ((vbuf = iter->next(&vsiz)) != NULL) {
			// 	doc_type = kyotocabinet::readfixnum(vbuf,4);
			// 	doc_id = kyotocabinet::readfixnum(vbuf+4,4);
			// 	new_docs.push_back(doc_pair(doc_type,doc_id));
			// }
			// string existing_docs;
			// string merged_docs;
			// registry_.indexDB->get(hash,&existing_docs);
			// // merge_docs(new_docs,existing_docs,merged_docs);
			// if (existing_docs.compare(merged_docs)!=0){
			// 	registry_.indexDB->set(hash,merged_docs);				
			// }
			return true;
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

		bool batch(Document& document){//string& job_name
			// Job job(registry_);
			// job.addItem(document.key());
			// Indexer indexer(registry_,job);     
			// // indexer.tune_storage(40,0,0);
			// kc::BasicDB* idb = registry_.jobDB->reveal_inner_db();
			// return job.start() && indexer.execute(idb,registry_.map_reduce_path,kyotocabinet::MapReduce::XNOLOCK|kyotocabinet::MapReduce::XNOCOMP) && job.finish();
			return true;
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
	};
}
#endif