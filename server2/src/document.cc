#include "document.h"

namespace superfastmatch
{
	Document::Document(const uint32_t doctype,const uint32_t docid,const char* content,const Registry& registry):
		doctype_(doctype),docid_(docid),registry_(registry),key_(0),content_(0),content_map_(0),hashes_(0),unique_sorted_hashes_(0),bloom_(0)
	{
		char key[8];
		kc::writefixnum(key,kc::hton32(doctype_),4);
		kc::writefixnum(key+4,kc::hton32(docid_),4);
		key_ = new string(key,8);
		content_ = new std::string(content);
	}
	
	Document::Document(string& key,const Registry& registry):
	registry_(registry),key_(0),content_(0),content_map_(0),hashes_(0),unique_sorted_hashes_(0),bloom_(0)
	{
		key_= new string(key);
		doctype_ = kc::ntoh32(kc::readfixnum(key.substr(0,4).data(),4));
		docid_ = kc::ntoh32(kc::readfixnum(key.substr(4,4).data(),4));
		content_ = new string();
		registry_.documentDB->get(*key_,content_);
	}
	
	Document::~Document(){
		clear();
		if(key_!=0){
			delete key_;
			key_=0;
		}
		if (content_!=0){
			delete content_;				
			content_=0;
		}
		// printf("Destroyed Document (%p)\n", this);
	}
	
	void Document::clear(){
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
	}

	bool Document::save(){
		return registry_.documentDB->cas(key_->data(),key_->size(),NULL,0,content_->data(),content_->size());
	}
	
	bool Document::load(){
		return registry_.documentDB->get(*key_,content_);
	}
	
	bool Document::remove(){
		return registry_.documentDB->remove(*key_);
	}
	
	void Document::serialize(stringstream& s){
		for (content_map::iterator it=content().begin();it!=content().end();it++){
			if (it->first!="text"){
				s << it->first << " : " << it->second << endl;	
			}
		}
		s << text();
	}
	
	Document::hashes_vector& Document::hashes(){
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
	
	Document::hashes_vector& Document::unique_sorted_hashes(){
		if (unique_sorted_hashes_==0){
			unique_sorted_hashes_=new hashes_vector();
			uint32_t length = text().length()-registry_.window_size;
			unique_sorted_hashes_->resize(length);
			const char* data = text().data();
			hash_t hash;
			for (uint32_t i=0;i<length;i++){
				hash = hashmurmur(data+i,registry_.window_size+1);
				(*unique_sorted_hashes_)[i]=hash;
			}
			std::sort(unique_sorted_hashes_->begin(),unique_sorted_hashes_->end());
			hashes_vector::iterator it = unique (unique_sorted_hashes_->begin(), unique_sorted_hashes_->end());
		  	unique_sorted_hashes_->resize(it-unique_sorted_hashes_->begin());
		}
		return *unique_sorted_hashes_;
	}
	
	Document::hashes_bloom& Document::bloom(){
		hashes();
		return *bloom_;
	}
	
	Document::content_map& Document::content(){
		if (content_map_==0){
			content_map_ = new content_map();
			kyototycoon::wwwformtomap(*content_,content_map_);
		}
		return *content_map_;
	}
	
	string& Document::text(){
		return content()["text"];
	}
	
	string& Document::key(){
		return *key_;
	}
	
	uint64_t Document::index_key(){
		uint64_t index_key=doctype_;
		return index_key<<32 | docid_;
	}
		
	uint32_t Document::windowsize(){
		return registry_.window_size;
	}
	
	uint32_t Document::doctype(){
		return doctype_;
	}
			
	uint32_t Document::docid(){
		return docid_;
	}	
		
	ostream& operator<< (ostream& stream, Document& document) {
		stream << "Document(" << document.doctype() << "," << document.docid() << ")";
		return stream;
	}
	
	bool operator< (Document& lhs,Document& rhs){
		if (lhs.doctype() == rhs.doctype()){
			return lhs.docid() < rhs.docid();
		} 
		return lhs.doctype() < rhs.doctype();
	}

}//namespace Superfastmatch