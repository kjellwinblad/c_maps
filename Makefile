
CC = gcc
CFLAGS = -I. -O3 -Wall -g
TEST_OBJECTS = test_skiplist.o skiplist.o
BENCHMARK_OBJECTS = benchmark_skiplist.o skiplist.o
LIBS =

all: test_skiplist benchmark_skiplist

test_skiplist: $(TEST_OBJECTS) 
	$(CC) -o test_skiplist $(TEST_OBJECTS) $(LIBS)

benchmark_skiplist: $(BENCHMARK_OBJECTS) 
	$(CC) -o benchmark_skiplist $(BENCHMARK_OBJECTS) $(LIBS)

run_test_skiplist: test_skiplist
	valgrind --leak-check=yes ./test_skiplist

run_benchmark_skiplist: benchmark_skiplist
	./benchmark_skiplist 1000000
	cd benchmark ; \
	gnuplot < plot_bench.gp

test_skiplist.o: test_skiplist.c skiplist.o
	$(CC) $(CFLAGS) -c test_skiplist.c

benchmark_skiplist.o: benchmark_skiplist.c skiplist.o
	$(CC) $(CFLAGS) -c benchmark_skiplist.c

skiplist.o: skiplist.c skiplist.h
	$(CC) $(CFLAGS) -c skiplist.c 

clean:
	rm -f test_skiplist benchmark_skiplist $(TEST_OBJECTS) $(BENCHMARK_OBJECTS)

check: test_skiplist benchmark_skiplist