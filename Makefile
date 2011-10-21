# Makefile for SuperFastMatch

#================================================================
# Setting Variables
#================================================================

# Generic settings
SHELL = /bin/sh

# Data path
DATA = ./data

# Targets
MYBINS = superfastmatch
OBJS =  src/worker.o src/queue.o src/posting.o src/document.o src/logger.o src/registry.o src/command.o src/postline.o src/association.o src/query.o src/task.o src/codec.o
MAIN = src/superfastmatch.o

# Building binaries
INCLUDES = -Isrc -Itests -I/usr/local/ -Itests/utils/
#LDFLAGS = -Wl,-no_pie
CXXFLAGS = -Wall -Wextra -funsigned-char -m64 -march=core2 -O3 -g
# CXXFLAGS = -Wall -Wextra -funsigned-char -msse4.1 -ftree-vectorize -O3 -g
#CXXFLAGS = -Wall -Wextra -funsigned-char -fno-omit-frame-pointer -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -m64 -march=core2 -O3 -g
LIBS =  -lstdc++ -lz -lm -lc -lpthread -lkyototycoon -lkyotocabinet -lctemplate -lgflags
# LIBS = -lkyototycoon -lkyotocabinet -lstdc++ -lz -lpthread -lm -lc -lctemplate -lgflags -ltcmalloc -lprofiler
CXX = g++ $(INCLUDES)
#CXX = icc $(INCLUDES)

# Enviroments
# RUNENV = TCMALLOC_SAMPLE_PARAMETER=524288
PROFILEENV = HEAPPROFILE=/tmp/superfastmatch.hprof 
DEBUGENV = gdb 

#================================================================
# Test variables
#================================================================

TESTS = tests/command-unittest.o tests/postline-unittest.o tests/document-unittest.o tests/association-unittest.o tests/posting-unittest.o tests/benchmark.o tests/hash-unittest.o tests/query-unittest.o
GTEST_DIR = tests/utils

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

all : $(MYBINS) 

clean :
	rm -rf $(MYBINS) $(DATA) tests/*.o $(GTEST_DIR)/*.o $(GTEST_DIR)/*.a *.a *.o *.exe src/*.o src/*.d
	mkdir -p $(DATA)

run : all
	mkdir -p $(DATA)
	$(RUNENV) ./superfastmatch -reset -debug

check : tests/tests
	tests/tests --gtest_filter=-*Slow*

profile : all
	mkdir -p $(DATA)
	$(PROFILEENV) ./superfastmatch -reset -debug

debug : CXXFLAGS += -O0 
debug : all
	mkdir -p data
	$(DEBUGENV) superfastmatch

.PHONY : all clean check profile debug run

#================================================================
# Building Tests
#================================================================

tests/gmock-gtest.a : $(GTEST_DIR)/gmock-gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

tests/tests : $(OBJS) $(TESTS) tests/tests.o tests/mock_registry.o tests/gmock-gtest.a
	$(CXX) $(INCLUDES) $(CXXFLAGS) -o $@ $^ $(LIBS)

#================================================================
# Building binaries
#================================================================

superfastmatch : $(OBJS) $(MAIN)
	$(CXX) $(OBJS) $(LDFLAGS) $(LIBS) $(MAIN) -o $@ 

# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.d)

# END OF FILE
# DO NOT DELETE
