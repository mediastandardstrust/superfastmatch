#ifndef _SFMDOCUMENT_H                       // duplication check
#define _SFMDOCUMENT_H

#include <vector>
#include <bitset>
#include <iostream>
#include <string>
#include <algorithm>
#include <common.h>
#include <kttimeddb.h>

namespace superfastmatch
{
	class Document
	{
	public:
		typedef std::vector<hash_t> hashes_vector;
		typedef std::bitset<(1<<24)> hashes_bloom;
		
	private:
		uint32_t m_doctype;
		uint32_t m_docid;
		uint32_t m_windowsize;
		std::string* m_key;
		std::string* m_text;
		hashes_vector* m_hashes;
		hashes_bloom* m_bloom;
		
	public:
		Document(uint32_t doctype,uint32_t docid,const char* text,uint32_t windowsize):
			m_key(0),m_text(0),m_hashes(0),m_bloom(0)
		{
			m_doctype=doctype;
			m_docid=docid;
			m_windowsize=windowsize;
			char key[8];
			kc::writefixnum(key,kc::hton32(m_doctype),4);
			kc::writefixnum(key+4,kc::hton32(m_docid),4);
			m_key = new string(key,8);
			m_text = new std::string(text);
		}
		
		~Document(){
			if(m_key!=0){
				delete m_key;
				m_key=0;
			}
			if (m_text!=0){
				delete m_text;				
				m_text=0;
			}
			if (m_hashes!=0){
				delete m_hashes;				
				m_hashes=0;
			}
			if (m_bloom!=0){
				delete m_bloom;
				m_bloom=0;	
			}
			// printf("Destroyed Document (%p)\n", this);
		}
		
		bool save(kyototycoon::TimedDB* db){
			return db->set(*m_key,*m_text);
		}
		
		bool load(kyototycoon::TimedDB* db){
			return db->get(*m_key,m_text);
		}
		
		bool remove(kyototycoon::TimedDB* db){
			return db->remove(*m_key);
		}
		
		hashes_vector& hashes(){
			if (m_hashes->size()==0){
				uint32_t length = m_text->length()-m_windowsize;
				m_hashes->resize(length);
				const char* data = m_text->data();
				hash_t hash;
				for (uint32_t i=0;i<length;i++){
					hash = hashmurmur(data+i,m_windowsize+1);
					(*m_hashes)[i]=hash;
					m_bloom->set(hash&0xFFFFFF);
				}
			}
			return *m_hashes;
		}
		
		hashes_bloom& bloom(){
			hashes();
			return *m_bloom;
		}
		
		std::string& text(){
			return *m_text;
		}
		
		uint32_t windowsize(){
			return m_windowsize;
		}
				
		friend std::ostream& operator<< (std::ostream& stream, const Document& document) {
			stream << "Document(" << document.m_doctype << "," << document.m_docid <<")";
			return stream;
		}
	};
}//namespace Superfastmatch

#endif                                   // duplication check