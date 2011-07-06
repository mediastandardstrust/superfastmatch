# Makefile for SuperFastMatch

#================================================================
# Setting Variables
#================================================================

# Generic settings
SHELL = /bin/sh

# Targets
MYBINS = superfastmatch
OBJS = src/superfastmatch.o src/worker.o src/queue.o src/posting.o src/document.o src/logger.o src/registry.o src/command.o
EXTERNAL = leveldb

# Building binaries
CXX = g++
CXXFLAGS = -c -I./src -I/usr/local/include/ -Iexternal/leveldb/include/ -Wall -funsigned-char -fno-omit-frame-pointer -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free  -m64 -march=core2 -O3 -g
LDFLAGS = -Lexternal/leveldb -lkyototycoon -lkyotocabinet -lleveldb -lstdc++ -lz -lpthread -lm -lc -ltcmalloc -lctemplate 

# Enviroments
RUNENV = DYLD_FORCE_FLAT_NAMESPACE=1 DYLD_INSERT_LIBRARIES=/usr/local/lib/libtcmalloc.dylib TCMALLOC_SAMPLE_PARAMETER=524288
PROFILEENV = DYLD_FORCE_FLAT_NAMESPACE=1 DYLD_INSERT_LIBRARIES=/usr/local/lib/libtcmalloc.dylib HEAPPROFILE=/tmp/superfastmatch.hprof 
# Donny specific (change to match your environment) will eventually put in external folder
DEBUGENV = DYLD_FORCE_FLAT_NAMESPACE=1 DYLD_LIBRARY_PATH=/Volumes/1TB/src/google/google-perftools-1.7/.libs/ gdb 
# DEBUGENV = DYLD_FORCE_FLAT_NAMESPACE=1 DYLD_LIBRARY_PATH=/Volumes/1TB/src/google/google-perftools-1.7/.libs/ HEAPPROFILE=/tmp/superfastmatch.hprof HEAP_PROFILE_MMAP=true gdb

#================================================================
# Suffix rules
#================================================================

.SUFFIXES : 
.SUFFIXES : .cc .o

.cc.o :
	$(CXX) $(CXXFLAGS) $< -o $@

#================================================================
# Dependency stuff
# http://www.scottmcpeak.com/autodepend/autodepend.html
#================================================================

%.o: %.cc
	$(CXX) -c $(CXXFLAGS) $*.cc -o $*.o
	$(CXX) -MM $(CXXFLAGS) $*.cc > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

#================================================================
# Actions
#================================================================

all : $(EXTERNAL) $(MYBINS) 

clean :
	rm -rf $(MYBINS) *.exe src/*.o src/*.d *.kch *.kct *.kcf            

check : all
	rm -rf *.kct *.kch *.kcf
	$(RUNENV) ./superfastmatch
	#Run test.sh

profile : all
	$(PROFILEENV) ./superfastmatch

debug : CXXFLAGS += -O0 
debug : all
	rm -rf *.kct *.kch *.kcf
	$(DEBUGENV) superfastmatch

# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.d)

leveldb :
	cd external/leveldb && $(MAKE)

.PHONY : all clean check profile debug $(EXTERNAL)

#================================================================
# Building binaries
#================================================================

superfastmatch : $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) -o $@ 

# END OF FILE
# DO NOT DELETE
