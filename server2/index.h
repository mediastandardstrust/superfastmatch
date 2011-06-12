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

typedef vector<uint32_t> docids_vector;
typedef std::tr1::unordered_map<uint32_t,docids_vector> docs_map;

inline void read_docs(const char* docs_buf,const size_t docs_size,docs_map& docs) {
	uint32_t offset=0;
	uint64_t doc_type;
	uint64_t doc_id;
	uint64_t doc_count;
	while (offset<docs_size) {
		offset+=kyotocabinet::readvarnum(docs_buf+offset,docs_size-offset,&doc_type);
		offset+=kyotocabinet::readvarnum(docs_buf+offset,docs_size-offset,&doc_count);
		docids_vector doc_ids;
		for (uint32_t i=0;i<doc_count;i++){
			offset+=kyotocabinet::readvarnum(docs_buf+offset,docs_size-offset,&doc_id);
			doc_ids.push_back(doc_id);
		}
		docs[doc_type]=doc_ids;
	};
}

inline void write_docs(const docs_map& docs, string& out){
	uint32_t doc_count=0;
	for(docs_map::const_iterator it=docs.begin();it!=docs.end();++it){
		doc_count+=(*it).second.size();
	}
	//Allocate maximum possible memory required
	char op[10*((docs.size()*2)+doc_count)];
	size_t ol=0;
	for(docs_map::const_iterator it=docs.begin();it!=docs.end();++it){
		ol+=kyotocabinet::writevarnum(op+ol,(*it).first);
		ol+=kyotocabinet::writevarnum(op+ol,(*it).second.size());
		for (docids_vector::const_iterator it2=(*it).second.begin();it2!=(*it).second.end();it2++){
			ol+=kyotocabinet::writevarnum(op+ol,(*it2));
		}
	}
	out = string(op,ol);
}

inline void merge_docs(const docs_map& new_docs,const string& existing_docs,string& out){
	docs_map docs;
	docids_vector::const_iterator it2;
	docids_vector merged_docs;
	read_docs(existing_docs.data(),existing_docs.size(),docs);
	for (docs_map::const_iterator it = new_docs.begin();it!=new_docs.end();++it){
		merged_docs.resize(it->second.size()+docs[it->first].size());
		it2=set_union(it->second.begin(),it->second.end(),docs[it->first].begin(),docs[it->first].end(),merged_docs.begin());
		merged_docs.resize(it2-merged_docs.begin());
		docs[it->first].swap(merged_docs);
	}
	write_docs(docs,out);
}

inline void delete_docs(){
	
}


namespace superfastmatch
{
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
			uint32_t hash;
			uint32_t hash_size = sizeof(hash);
			string hash_string;
			string existing_docs;
			string merged_docs;
			docs_map new_docs;
			new_docs[document.doctype()].push_back(document.docid());
			Document::hashes_vector::const_iterator ite=document.unique_sorted_hashes().end();
			for (Document::hashes_vector::const_iterator it=document.unique_sorted_hashes().begin();it!=ite;++it){
				hash=kc::hton32(*it);
				hash_string=string((char*)&hash,hash_size);
				registry_.indexDB->get(hash_string,&existing_docs);
				merge_docs(new_docs,existing_docs,merged_docs);
				if (existing_docs.compare(merged_docs)!=0){
					registry_.indexDB->set(hash_string,merged_docs);				
				}	
			}
			return true;
		}
	};
}
#endif