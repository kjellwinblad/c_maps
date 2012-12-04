
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "assert.h"


#include "skiplist.h"

#define TO_VP(intValue) (void *)(intValue)


double benchmark_put(int num_of_operations){

    int i;

    clock_t begin, end;

    double time_spent;

    KVSet * skiplist = new_skiplist_default();

    void * elements[num_of_operations];

    
    srand((unsigned)time(0));

    //Create elements
    for(i = 0; i < num_of_operations; i++){
        elements[i] = TO_VP(0xFFFFFFFFFFFFFFFF & (rand()+1));
    }


    begin = clock();

    //Inserts
    for(i = 0; i < num_of_operations; i++){
        skiplist->funs.put(skiplist, elements[i]);
    }

    end = clock();
    time_spent = ((double)(end - begin));

    skiplist->funs.delete(skiplist, NULL);

    return time_spent;

}

double benchmark_lookup(int num_of_operations){

    int i;

    clock_t begin, end;

    double time_spent;

    KVSet * skiplist = new_skiplist_default();

    void * elements[num_of_operations];

    
    srand((unsigned)time(0));

    //Create elements
    for(i = 0; i < num_of_operations; i++){
        elements[i] = TO_VP(0xFFFFFFFFFFFFFFFF & (rand()+1));
    }

    //Inserts
    for(i = 0; i < num_of_operations; i++){
        skiplist->funs.put(skiplist, elements[i]);
    }

    begin = clock();

    //lookups
    for(i = 0; i < num_of_operations; i++){
        assert(elements[i] == skiplist->funs.lookup(skiplist, elements[i]));
    }
    end = clock();
    time_spent = ((double)(end - begin));

    skiplist->funs.delete(skiplist, NULL);

    return time_spent;

}


double benchmark_remove(int num_of_operations){

    int i;

    clock_t begin, end;

    double time_spent;

    KVSet * skiplist = new_skiplist_default();

    void * elements[num_of_operations];

    
    srand((unsigned)time(0));

    //Create elements
    for(i = 0; i < num_of_operations; i++){
        elements[i] = TO_VP(0xFFFFFFFFFFFFFFFF & (rand()+1));
    }

    //Inserts
    for(i = 0; i < num_of_operations; i++){
        skiplist->funs.put(skiplist, elements[i]);
    }

    begin = clock();

    //Removes
    for(i = 0; i < num_of_operations; i++){
        skiplist->funs.remove(skiplist, elements[i]);
    }
    end = clock();
    time_spent = ((double)(end - begin));

    skiplist->funs.delete(skiplist, NULL);

    return time_spent;

}


int main(int argc, char **argv){

    int max_num_of_operations;

    int i;

    double time;

    max_num_of_operations = atoi(argv[1]);

    FILE *benchmark_put_file;
    benchmark_put_file = fopen("benchmark/benchmark_put.dat","w");

    FILE *benchmark_lookup_file;
    benchmark_lookup_file = fopen("benchmark/benchmark_lookup.dat","w");

    FILE *benchmark_remove_file;
    benchmark_remove_file = fopen("benchmark/benchmark_remove.dat","w");

    fprintf(stderr, "\n\n\033[32m -- STARTING BENCHMARKS! -- \033[m\n\n");
            
    for(i = 0 ; i <= max_num_of_operations; i = i + 20000){
        //Put
        printf("benchmark_put(%d)\n", i);
        time = benchmark_put(i);
        fprintf(benchmark_put_file, "%d %f\n", i, time / i);
        printf("ready benchmark_put(%d)\n", i);
        //Lookup
        printf("benchmark_lookup(%d)\n", i);
        time = benchmark_lookup(i);
        fprintf(benchmark_lookup_file, "%d %f\n", i, time / i);
        printf("ready benchmark_lookup(%d)\n", i);
        //Remove
        printf("benchmark_remove(%d)\n", i);
        time = benchmark_remove(i);
        fprintf(benchmark_remove_file, "%d %f\n", i, time / i);
        printf("ready benchmark_remove(%d)\n", i);
    }


    printf("\n\n\033[32m -- BENCHMARKS COMPLETED! -- \033[m\n\n");

    exit(0);

}

