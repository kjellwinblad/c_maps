#include "stdio.h"
#include "skiplist.h"
#include "benchmark_kvset.h"

int main(int argc, char **argv){

    int max_num_of_operations;

    max_num_of_operations = atoi(argv[1]);

    run_benchmark(new_skiplist_default, max_num_of_operations, "skiplist");

    exit(0);
    
}

