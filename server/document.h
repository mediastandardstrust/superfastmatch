#ifndef _SFMDOCUMENT_H                       // duplication check
#define _SFMDOCUMENT_H

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "lunar.h"

namespace Superfastmatch
{
	class Document
	{
	public:
		static const char className[];
		static Lunar<Document>::RegType methods[];
		
		Document(lua_State *L){};
		int compare(lua_State *L){return 0;}
		~Document(){}
	};
	
	const char Document::className[] = "Document";
	
	Lunar<Document>::RegType Document::methods[] = {
	  LUNAR_DECLARE_METHOD(Document, compare),
	  {0,0}
	};
}//namespace Superfastmatch

#endif                                   // duplication check