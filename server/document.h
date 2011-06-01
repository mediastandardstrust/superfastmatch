#ifndef _SFMDOCUMENT_H                       // duplication check
#define _SFMDOCUMENT_H

#include "common.h"
#include <vector>
#include <bitset>
#include <iostream>
#include <string>

namespace Superfastmatch
{
	class Document
	{
	private:
		uint32_t m_doctype;
		uint32_t m_docid;
		uint32_t m_windowsize;
		std::string* m_text;
		std::vector<uint32_t>* m_hashes;
		std::bitset<(1<<24)>* m_bloom;
		
	public:
		//C++ interface
		std::vector<uint32_t>& hashes(){
			if (m_hashes->size()==0){
				uint32_t length = m_text->length()-m_windowsize;
				const char* data = m_text->data();
				uint32_t hash;
				for (uint32_t i=0;i<length;i++){
					hash = hashmurmur(data+i,m_windowsize+1);
					m_hashes->push_back(hash);
					m_bloom->set(hash&0xFFFFFF);
				}
			}
			return *m_hashes;
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
		
		//Lua Interface
		Document(lua_State *L)
		{
			m_doctype=luaL_checknumber(L,1);
			m_docid=luaL_checknumber(L,2);
			m_windowsize=luaL_checknumber(L,4);
			m_text=new std::string(luaL_checkstring(L,3));
			m_hashes = new std::vector<uint32_t>();
			m_bloom = new std::bitset<(1<<24)>();
		};
		int doctype(lua_State *L)	{lua_pushnumber(L,m_doctype);return 1;}
		int docid(lua_State *L)		{lua_pushnumber(L,m_docid);return 1;}
		int text(lua_State *L)		{lua_pushstring(L,m_text->data());return 1;}
		int hashes(lua_State *L)	{
										std::vector<uint32_t>&h = hashes();
										uint32_t l = h.size();
									 	lua_createtable(L,l, 0);
									 	uint32_t t = lua_gettop(L);
									 	for(uint32_t i=0; i < l; i++) {
											lua_pushnumber(L,h[i]);
	        								lua_rawseti(L, t, i + 1);
	    							 	}
										return 1;
									}
		int compare(lua_State *L){return 0;}		
		
		static const char className[];
		static Lunar<Document>::RegType methods[];

	};
	
	const char Document::className[] = "Document";
	
	Lunar<Document>::RegType Document::methods[] = {
	  LUNAR_DECLARE_METHOD(Document, doctype),
	  LUNAR_DECLARE_METHOD(Document, docid),
	  LUNAR_DECLARE_METHOD(Document, text),
	  LUNAR_DECLARE_METHOD(Document, hashes),
	  LUNAR_DECLARE_METHOD(Document, compare),
	  {0,0}
	};
}//namespace Superfastmatch

#endif                                   // duplication check