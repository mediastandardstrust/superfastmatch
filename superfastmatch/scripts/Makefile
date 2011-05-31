# Makefile for Lua SuperFastMatch -- a text comparison library for Lua 5.1.
# To compile with MSVC please run: msvcbuild.bat
# To compile with MinGW please run: mingw32-make -f Makefile.mingw

# Include path where lua.h, luaconf.h and lauxlib.h reside:
INCLUDES= -I/usr/include/lua5.1 -I/usr/local/include

DEFINES=

# Lua executable name. Used to find the install path and for testing.
LUA= lua

CC= g++
SOCFLAGS= -fPIC -lkyotocabinet -m64 -march=core2
SOCC= $(CC) -shared $(SOCFLAGS)
CXXFLAGS= -Wall -O3 $(SOCFLAGS) $(DEFINES) $(INCLUDES)
RM= rm -f
INSTALL= install -p
INSTALLPATH= $(LUA) installpath.lua

MODNAME= superfastmatch
MODSO= $(MODNAME).so

all: $(MODSO) 

debug:
	$(MAKE) all " $(CC) $(SOCFLAGS) -g -pg"

# Alternative target for compiling on Mac OS X:
macosx:
	$(MAKE) all "SOCC=MACOSX_DEPLOYMENT_TARGET=10.3 $(CC) -dynamiclib -single_module -undefined dynamic_lookup  $(SOCFLAGS)"

macosxdebug: 
	$(MAKE) all "SOCC=MACOSX_DEPLOYMENT_TARGET=10.3 $(CC) -dynamiclib -single_module -undefined dynamic_lookup $(SOCFLAGS) -g -pg "

$(MODSO): $(MODNAME).o 
	$(SOCC) -o $@ $< 

install: $(MODSO)
	$(INSTALL) $< `$(INSTALLPATH) $(MODNAME)`

test: $(MODSO)
	@$(LUA) test.lua && echo "basic test OK"

clean:
	$(RM) *.o *.so *.obj *.lib *.exp *.dll *.manifest

.PHONY: all macosx install test clean

