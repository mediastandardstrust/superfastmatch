#ifndef _SFMSUPERFASTMATCH_H                       // duplication check
#define _SFMSUPERFASTMATCH_H

#include "common.h"
#include "document.h"
namespace Superfastmatch
{
	extern "C"{ 
	   LUALIB_API int luaopen_superfastmatch(lua_State *L)
	   {
			Lunar<Document>::Register(L);
	     	return 1;
	   }
	}
}

#endif