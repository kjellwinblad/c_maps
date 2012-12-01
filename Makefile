
CC = gcc
CFLAGS = -I. -g -O0 -Wall
OBJECTS = test_skiplist.o skiplist.o
LIBS =

all: test_skiplist

test_skiplist: $(OBJECTS) 
	$(CC)  -o test_skiplist  $(OBJECTS) $(LIBS)

run_test_skiplist: test_skiplist
	valgrind --leak-check=yes ./test_skiplist

test_skiplist.o: test_skiplist.c skiplist.h
	$(CC) $(CFLAGS) -c test_skiplist.c

skiplist.o: skiplist.c skiplist.h
	$(CC) $(CFLAGS) -c skiplist.c 

clean:
	rm -f test_skiplist $(OBJECTS)

check: test_skiplist