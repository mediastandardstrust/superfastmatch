#ifndef _SFMDOCUMENT_H                       // duplication check
#define _SFMDOCUMENT_H

#include <vector>
#include <bitset>
#include <iostream>
#include <string>
#include <map>
#include <algorithm>
#include <common.h>
#include <ktutil.h>
#include <registry.h>

namespace superfastmatch
{	
	class Document
	{
	public:
		typedef std::vector<hash_t> hashes_vector;
		typedef std::bitset<(1<<24)> hashes_bloom;
		typedef std::map<std::string,std::string> content_map;
		
	private:
		uint32_t doctype_;
		uint32_t docid_;
		const Registry& registry_;
		std::string* key_;
		std::string* content_;
		content_map* content_map_;
		hashes_vector* hashes_;
		hashes_vector* unique_sorted_hashes_;
		hashes_bloom* bloom_;

		
	public:
		Document(const uint32_t doctype,const uint32_t docid,const char* content,const Registry& registry):
			doctype_(doctype),docid_(docid),registry_(registry),key_(0),content_(0),content_map_(0),hashes_(0),unique_sorted_hashes_(0),bloom_(0)
		{
			char key[8];
			kc::writefixnum(key,kc::hton32(doctype_),4);
			kc::writefixnum(key+4,kc::hton32(docid_),4);
			key_ = new string(key,8);
			content_ = new std::string(content);
		}
		
		Document(string& key,const Registry& registry):
		registry_(registry),key_(0),content_(0),content_map_(0),hashes_(0),unique_sorted_hashes_(0),bloom_(0)
		{
			key_= new string(key);
			doctype_ = kc::ntoh32(kc::readfixnum(key.substr(0,4).data(),4));
			docid_ = kc::ntoh32(kc::readfixnum(key.substr(4,4).data(),4));
			content_ = new string();
			registry_.documentDB->get(*key_,content_);
		}
		
		~Document(){
			if(key_!=0){
				delete key_;
				key_=0;
			}
			if (content_!=0){
				delete content_;				
				content_=0;
			}
			if (content_map_!=0){
				delete content_map_;
				content_map_=0;
			}
			if (hashes_!=0){
				delete hashes_;				
				hashes_=0;
			}
			if (unique_sorted_hashes_!=0){
				delete unique_sorted_hashes_;
				unique_sorted_hashes_=0;
			}
			if (bloom_!=0){
				delete bloom_;
				bloom_=0;	
			}
			// printf("Destroyed Document (%p)\n", this);
		}
		
		bool save(){
			return registry_.documentDB->set(*key_,*content_);
		}
		
		bool load(){
			return registry_.documentDB->get(*key_,content_);
		}
		
		bool remove(){
			return registry_.documentDB->remove(*key_);
		}
		
		void serialize(stringstream& s){
			for (content_map::iterator it=content().begin();it!=content().end();it++){
				if (it->first!="text"){
					s << it->first << " : " << it->second << endl;	
				}
			}
			s << text();
		}
		
		hashes_vector& hashes(){
			if (hashes_==0){
				hashes_ = new hashes_vector();
				bloom_ = new hashes_bloom();
				uint32_t length = text().length()-registry_.window_size;
				hashes_->resize(length);
				const char* data = text().data();
				hash_t hash;
				for (uint32_t i=0;i<length;i++){
					hash = hashmurmur(data+i,registry_.window_size+1);
					(*hashes_)[i]=hash;
					bloom_->set(hash&0xFFFFFF);
				}
			}
			return *hashes_;
		}
		
		hashes_vector& unique_sorted_hashes(){
			if (unique_sorted_hashes_==0){
				unique_sorted_hashes_=new hashes_vector(hashes());
				std::sort(unique_sorted_hashes_->begin(),unique_sorted_hashes_->end());
				hashes_vector::iterator it = unique (unique_sorted_hashes_->begin(), unique_sorted_hashes_->end());
			  	unique_sorted_hashes_->resize(it-unique_sorted_hashes_->begin());
			}
			return *unique_sorted_hashes_;
		}
		
		hashes_bloom& bloom(){
			hashes();
			return *bloom_;
		}
		
		std::string& text(){
			return content()["text"];
		}
		
		std::string& key(){
			return *key_;
		}
		
		uint64_t index_key(){
			uint64_t index_key=doctype_;
			return index_key<<32 | docid_;
		}
		
		content_map& content(){
			if (content_map_==0){
				content_map_ = new content_map();
				kyototycoon::wwwformtomap(*content_,content_map_);
			}
			return *content_map_;
		}
		
		uint32_t windowsize(){
			return registry_.window_size;
		}
		
		uint32_t doctype(){
			return doctype_;
		}
				
		uint32_t docid(){
			return docid_;
		}	
			
		friend std::ostream& operator<< (std::ostream& stream, Document& document) {
			stream << "Document(" << document.doctype() << "," << document.docid() << ")";
			return stream;
		}
	};
}//namespace Superfastmatch

#endif                                   // duplication check