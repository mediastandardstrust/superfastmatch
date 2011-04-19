CC = g++
CPPFLAGS = -m64 -Wall -g -O2
LDFLAGS	 = -lPocoFoundation -lkyotocabinet -lz -lstdc++ -lpthread -lm -lc

merge	:	merge.cpp
	$(CC) $(CPPFLAGS) ${LDFLAGS}  $< -o $@

clean	:
	rm ./merge
