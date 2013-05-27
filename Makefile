
CC = gcc
CFLAGS = -I. -Ihazard_pointer_lib -O0 -Wall -g -Wdeclaration-after-statement -pthread
TEST_OBJECTS_SKIPLIST = test_skiplist.o test_kvset.o  skiplist.o
TEST_OBJECTS_SKIPLIST_CONCURRENT = test_skiplist.o test_kvset.o skiplist_concurrent.o hazard_pointers.o
BENCHMARK_SKIPLIST_OBJECTS = benchmark_skiplist.o skiplist.o benchmark_kvset.o
LIBS =
#test_skiplist benchmark_skiplist 
all: test_skiplist_concurrent hazard_pointers.o

test_skiplist: $(TEST_OBJECTS_SKIPLIST) 
	$(CC) -o test_skiplist $(TEST_OBJECTS_SKIPLIST) $(CFLAGS) $(LIBS)

test_skiplist_concurrent: $(TEST_OBJECTS_SKIPLIST_CONCURRENT) 
	$(CC) -o test_skiplist_concurrent $(TEST_OBJECTS_SKIPLIST_CONCURRENT) $(CFLAGS) $(LIBS)

benchmark_skiplist: $(BENCHMARK_SKIPLIST_OBJECTS) 
	$(CC) -o benchmark_skiplist $(BENCHMARK_SKIPLIST_OBJECTS) $(CFLAGS) $(LIBS)

run_test_skiplist: test_skiplist
	valgrind --leak-check=yes ./test_skiplist

run_benchmark_skiplist: benchmark_skiplist
	./benchmark_skiplist 1000000
	cd benchmark ; \
	gnuplot < plot_bench.gp

test_kvset.o: test_kvset.c test_kvset.h kvset.h
	$(CC) $(CFLAGS) -c test_kvset.c

test_skiplist.o: test_skiplist.c skiplist.o test_kvset.o
	$(CC) $(CFLAGS) -c test_skiplist.c

benchmark_kvset.o: benchmark_kvset.c benchmark_kvset.h kvset.h
	$(CC) $(CFLAGS) -c benchmark_kvset.c

benchmark_skiplist.o: benchmark_skiplist.c skiplist.o benchmark_kvset.o
	$(CC) $(CFLAGS) -c benchmark_skiplist.c

skiplist.o: skiplist.c skiplist.h kvset.h
	$(CC) $(CFLAGS) -c skiplist.c 

skiplist_concurrent.o: skiplist_concurrent.c skiplist_concurrent.h kvset.h
	$(CC) $(CFLAGS) -c skiplist_concurrent.c 

hazard_pointers.o: hazard_pointers.c hazard_pointers.h
	$(CC) $(CFLAGS) -c hazard_pointers.c 

clean:
	rm -f test_skiplist test_skiplist_concurrent benchmark_skiplist $(TEST_OBJECTS_SKIPLIST) $(TEST_OBJECTS_SKIPLIST_CONCURRENT) $(BENCHMARK_SKIPLIST_OBJECTS)

check: test_skiplist benchmark_skiplist
