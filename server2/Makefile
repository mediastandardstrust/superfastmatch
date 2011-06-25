# Makefile for SuperFastMatch

#================================================================
# Setting Variables
#================================================================

# Generic settings
SHELL = /bin/sh

# Targets
MYBINS = superfastmatch
OBJS = src/superfastmatch.o src/histogram.o src/worker.o src/queue.o src/posting.o src/document.o src/logger.o src/registry.o src/command.o
EXTERNAL = leveldb

# Building binaries
CXX = g++
CXXFLAGS = -c -I./src -I/usr/local/include/ -Iexternal/leveldb/include/ -Wall -fsigned-char -fno-omit-frame-pointer -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -O3 -g -m64 -march=core2
LDFLAGS = -Lexternal/leveldb -lkyototycoon -lkyotocabinet -lleveldb -lstdc++ -lz -lpthread -lm -lc -ltcmalloc

# Enviroments
RUNENV = TCMALLOC_SAMPLE_PARAMETER=524288
PROFILEENV = DYLD_FORCE_FLAT_NAMESPACE=1 HEAPPROFILE=/tmp/superfastmatch.hprof 
# Donny specific (change to match your environment) will eventually put in external folder
DEBUGENV = DYLD_LIBRARY_PATH=/Volumes/1TB/src/google/google-perftools-1.7/.libs/ gdb 

#================================================================
# Suffix rules
#================================================================

.SUFFIXES : 
.SUFFIXES : .cc .o

.cc.o :
	$(CXX) $(CXXFLAGS) $< -o $@

#================================================================
# Actions
#================================================================

all : $(EXTERNAL) $(MYBINS) 

clean :
	rm -rf $(MYBINS) *.exe src/*.o *.kch *.kct

check : all
	rm -rf *.kct *.kch
	$(RUNENV) ./superfastmatch
	#Run test.sh

profile : all
	$(PROFILEENV) ./superfastmatch

debug : CXXFLAGS += -O0 
debug : clean all
	$(DEBUGENV) superfastmatch

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
