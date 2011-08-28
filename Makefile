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
OBJS = src/superfastmatch.o src/worker.o src/queue.o src/posting.o src/document.o src/logger.o src/registry.o src/command.o src/postline.o src/association.o

# Building binaries
INCLUDES = -I./src -I./tests -I/usr/local/include/ -Itests/utils/
#LDFLAGS = -Wl,-no_pie
CXXFLAGS = -Wall -Wextra -funsigned-char -m64 -march=core2 -O3 -g
#CXXFLAGS = -Wall -Wextra -funsigned-char -fno-omit-frame-pointer -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -m64 -march=core2 -O3 -g
LIBS = -lkyototycoon -lkyotocabinet -lstdc++ -lz -lpthread -lm -lc -lctemplate -lgflags
# LIBS = -lkyototycoon -lkyotocabinet -lstdc++ -lz -lpthread -lm -lc -lctemplate -lgflags -ltcmalloc -lprofiler
CXX = g++ $(INCLUDES)

# Enviroments
# RUNENV = TCMALLOC_SAMPLE_PARAMETER=524288
PROFILEENV = HEAPPROFILE=/tmp/superfastmatch.hprof 
DEBUGENV = gdb 

#================================================================
# Test variables
#================================================================

TESTS = tests/postline-unittest.o tests/document-unittest.o tests/association-unittest.o tests/posting-unittest.o tests/benchmark.o
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

check : CXXFLAGS += -O0
check : $(TESTS)
	$(TESTS:.o=;)

run : all
	mkdir -p $(DATA)
	$(RUNENV) ./superfastmatch -reset -debug

profile : all
	mkdir -p $(DATA)
	$(PROFILEENV) ./superfastmatch -reset -debug

debug : CXXFLAGS += -O0 
debug : all
	mkdir -p data
	$(DEBUGENV) superfastmatch

# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.d)

.PHONY : all clean check profile debug run

#================================================================
# Building Tests
#================================================================

gmock-gtest.a : $(GTEST_DIR)/gmock-gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

tests/postline-unittest.o : src/postline.cc tests/postline-unittest.cc gmock-gtest.a 
	$(CXX) -lpthread $(CXXFLAGS) $^ -o $* 

tests/document-unittest.o : src/document.cc src/association.cc tests/document-unittest.cc gmock-gtest.a
	$(CXX) $(INCLUDES) -lpthread -lkyotocabinet -lctemplate $(CXXFLAGS) $^ -o $*

tests/association-unittest.o : src/document.cc src/association.cc src/logger.cc tests/association-unittest.cc gmock-gtest.a
	$(CXX) $(INCLUDES) -lpthread -lkyotocabinet -lkyototycoon -lctemplate $(CXXFLAGS) $^ -o $*

tests/posting-unittest.o : src/document.cc src/posting.cc src/logger.cc src/association.cc src/postline.cc src/command.cc tests/posting-unittest.cc gmock-gtest.a
	$(CXX) $(INCLUDES) -lpthread -lkyotocabinet -lkyototycoon -lctemplate $(CXXFLAGS) $^ -o $*

tests/benchmark.o : src/document.cc src/posting.cc src/logger.cc src/association.cc src/postline.cc src/command.cc tests/benchmark.cc gmock-gtest.a
	$(CXX) $(INCLUDES) -lpthread -lkyotocabinet -lkyototycoon -lctemplate $(CXXFLAGS) $^ -o $*

#================================================================
# Building binaries
#================================================================

superfastmatch : $(OBJS) 
	$(CXX) $(LDFLAGS) $(LIBS) $(OBJS) -o $@ 

# END OF FILE
# DO NOT DELETE
