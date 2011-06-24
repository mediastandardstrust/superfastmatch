# Makefile for sample programs of Kyoto Tycoon


#================================================================
# Setting Variables
#================================================================


# Generic settings
SHELL = /bin/sh

# Targets
MYBINS = superfastmatch
OBJS = 

# Building binaries
CC = gcc
CXX = g++
# CFLAGS = -I. -I.. -Wall -ansi -pedantic -fsigned-char -O3
CXXFLAGS = -I. -I.. -Wall -fsigned-char -fno-omit-frame-pointer -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -O3 -g -m64 -march=core2
# CXXFLAGS = -I. -I.. -Wall -fsigned-char -O3 -g -m64 -march=core2
LDFLAGS =
# LIBS = -L. -L.. -lkyototycoon -lkyotocabinet -lstdc++ -lz -lpthread -lm -lc
LIBS = -L. -L.. -lkyototycoon -lkyotocabinet -lstdc++ -lz -lpthread -lm -lc -ltcmalloc
LDENV = LD_RUN_PATH=/lib:/usr/lib:$(HOME)/lib:/usr/local/lib:.:..
# RUNENV = DYLD_FORCE_FLAT_NAMESPACE=1 TCMALLOC_RELEASE_RATE=10 HEAPPROFILE=/tmp/superfastmatch.hprof LD_LIBRARY_PATH=/lib:/usr/lib:$(HOME)/lib:/usr/local/lib:.:..
RUNENV = LD_LIBRARY_PATH=/lib:/usr/lib:$(HOME)/lib:/usr/local/lib:.:..



#================================================================
# Suffix rules
#================================================================


.SUFFIXES :
.SUFFIXES : .c .cc .o

.c.o :
	$(CC) -c $(CFLAGS) $<

.cc.o :
	$(CXX) -c $(CXXFLAGS) $<



#================================================================
# Actions
#================================================================


all : $(MYBINS)

clean :
	rm -rf $(MYBINS) *.exe *.o a.out check.out gmon.out leak.log *.kch *.kct *~


static :
	make LDFLAGS="$(LDFLAGS) -static"


check :
	rm -rf casket*
	$(RUNENV) ./superfastmatch


.PHONY : all clean static



#================================================================
# Building binaries
#================================================================

superfastmatch : superfastmatch.o
	$(LDENV) $(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)


# END OF FILE
# DO NOT DELETE
