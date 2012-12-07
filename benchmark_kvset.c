#include "benchmark_kvset.h"

#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "assert.h"


#include "skiplist.h"

/*
 * ==================
 * Internal functions
 * ==================
 */

#define TO_VP(intValue) (void *)(intValue)


double benchmark_put(KVSet * (*create_kvset_fun)(), int num_of_operations){

    int i;

    clock_t begin, end;

    double time_spent;

    KVSet * skiplist = create_kvset_fun();

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

    skiplist->funs.delete_table(skiplist, NULL);

    return time_spent;

}

double benchmark_lookup(KVSet * (*create_kvset_fun)(), int num_of_operations){

    int i;

    clock_t begin, end;

    double time_spent;

    KVSet * skiplist = create_kvset_fun();

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

    skiplist->funs.delete_table(skiplist, NULL);

    return time_spent;

}


double benchmark_remove(KVSet * (*create_kvset_fun)(), int num_of_operations){

    int i;

    clock_t begin, end;

    double time_spent;

    KVSet * skiplist = create_kvset_fun();

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

    skiplist->funs.delete_table(skiplist, NULL);

    return time_spent;

}


/*
 * ================
 * Public interface
 * ================
 */

void run_benchmark(KVSet * (*create_kvset_fun)(),
                   int max_num_of_operations,
                   char benchmark_identifier[]){

    int i;

    double time;

    char * file_name_buffer = malloc(4096);
    char file_name_format [] = "benchmark/benchmark_%s_%s.dat";

    FILE *benchmark_put_file;
    FILE *benchmark_lookup_file;
    FILE *benchmark_remove_file;

    sprintf(file_name_buffer, file_name_format, benchmark_identifier, "put");
    benchmark_put_file = fopen(file_name_buffer, "w");

    sprintf(file_name_buffer, file_name_format, benchmark_identifier, "lookup");
    benchmark_lookup_file = fopen(file_name_buffer, "w");

    sprintf(file_name_buffer, file_name_format, benchmark_identifier, "remove");
    benchmark_remove_file = fopen(file_name_buffer, "w");

    free(file_name_buffer);

    fprintf(stderr, "\n\n\033[32m -- STARTING BENCHMARKS FOR %s! -- \033[m\n\n", benchmark_identifier);
            
    for(i = 0 ; i <= max_num_of_operations; i = i + 20000){
        //Put
        printf("benchmark_put(%d)\n", i);
        time = benchmark_put(create_kvset_fun, i);
        fprintf(benchmark_put_file, "%d %f\n", i, time / i);
        printf("ready benchmark_put(%d)\n", i);
        //Lookup
        printf("benchmark_lookup(%d)\n", i);
        time = benchmark_lookup(create_kvset_fun, i);
        fprintf(benchmark_lookup_file, "%d %f\n", i, time / i);
        printf("ready benchmark_lookup(%d)\n", i);
        //Remove
        printf("benchmark_remove(%d)\n", i);
        time = benchmark_remove(create_kvset_fun, i);
        fprintf(benchmark_remove_file, "%d %f\n", i, time / i);
        printf("ready benchmark_remove(%d)\n", i);
    }


    fclose(benchmark_put_file);
    fclose(benchmark_lookup_file);
    fclose(benchmark_remove_file);

    printf("\n\n\033[32m -- BENCHMARKS FOR %s COMPLETED! -- \033[m\n\n", benchmark_identifier);



}


