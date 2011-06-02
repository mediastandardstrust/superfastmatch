#ifndef _SFMDOCUMENT_H                       // duplication check
#define _SFMDOCUMENT_H

#include "common.h"
#include <vector>
#include <bitset>
#include <iostream>
#include <string>
#include <algorithm>

namespace Superfastmatch
{
	class Document
	{
	public:
		typedef std::vector<uint32_t> hashes_vector;
		typedef std::bitset<(1<<24)> hashes_bloom;
		
	private:
		uint32_t m_doctype;
		uint32_t m_docid;
		uint32_t m_windowsize;
		std::string* m_text;
		hashes_vector* m_hashes;
		hashes_bloom* m_bloom;
		
	public:
		//C++ interface
		hashes_vector& hashes(){
			if (m_hashes->size()==0){
				uint32_t length = m_text->length()-m_windowsize;
				m_hashes->resize(length);
				const char* data = m_text->data();
				uint32_t hash;
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
				
		~Document(){
			delete m_text;
			delete m_hashes;
			delete m_bloom;
			m_text=0;
			m_hashes=0;
			m_bloom=0;
			printf("deleted Document (%p)\n", this);
		}
		
		friend std::ostream& operator<< (std::ostream& stream, const Document& document) {
			stream << "Document(" << document.m_doctype << "," << document.m_docid <<")";
			return stream;
		}
		
		//Lua Interface
		Document(lua_State *L)
		{
			m_doctype=luaL_checkint(L,1);
			m_docid=luaL_checkint(L,2);
			m_windowsize=luaL_checkint(L,4);
			m_text=new std::string(luaL_checkstring(L,3));
			m_hashes = new hashes_vector();
			m_bloom = new hashes_bloom();
		};
		int doctype(lua_State *L)	{lua_pushinteger(L,m_doctype);return 1;}
		int docid(lua_State *L)		{lua_pushinteger(L,m_docid);return 1;}
		int text(lua_State *L)		{lua_pushstring(L,m_text->data());return 1;}
		int hashes(lua_State *L)	{
										hashes_vector&h = hashes();
										uint32_t l = h.size();
									 	lua_createtable(L,l, 0);
									 	uint32_t t = lua_gettop(L);
									 	for(uint32_t i=0; i < l; i++) {
											lua_pushnumber(L,h[i]);
	        								lua_rawseti(L, t, i + 1);
	    							 	}
										return 1;
									}
		
		static const char className[];
		static Lunar<Document>::RegType methods[];

	};
	
	const char Document::className[] = "Document";
	
	Lunar<Document>::RegType Document::methods[] = {
	  LUNAR_DECLARE_METHOD(Document, doctype),
	  LUNAR_DECLARE_METHOD(Document, docid),
	  LUNAR_DECLARE_METHOD(Document, text),
	  LUNAR_DECLARE_METHOD(Document, hashes),
	  {0,0}
	};
}//namespace Superfastmatch

#endif                                   // duplication check