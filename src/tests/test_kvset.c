#include "test_kvset.h"

#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "assert.h"
#include "pthread.h"


/*
 * ==================
 * Internal functions
 * ==================
 */


#define TO_VP(intValue) (void *)(intValue)

#define T(testFunCall, tesName)                 \
    printf("STARTING TEST: ");                  \
    test(testFunCall, tesName);

static
void test(int success, char msg[]){

    if(success){
        printf("\033[32m -- SUCCESS! -- \033[m");
    }else{
        printf("\033[31m -- FAIL! -- \033[m");
    }

    printf("TEST: %s\n", msg);

}



int test_create_and_delete(KVSet * (*create_kvset_fun)()){
    KVSet* skiplist = create_kvset_fun();
    skiplist->funs.delete_table(skiplist, NULL, NULL);
    return 1;
}

int test_insert(KVSet * (*create_kvset_fun)()){
    KVSet* skiplist = create_kvset_fun();

    void * el = skiplist->funs.put(skiplist, TO_VP(10));

    skiplist->funs.delete_table(skiplist, NULL, NULL);

    return el == NULL;
}

int test_insert_new(KVSet * (*create_kvset_fun)()){
    KVSet* skiplist = create_kvset_fun();
    
    int el2;
          
    int el1 = skiplist->funs.put_new(skiplist, TO_VP(10));

    assert(skiplist->funs.lookup(skiplist, TO_VP(10)) == TO_VP(10));

    el2 = skiplist->funs.put_new(skiplist, TO_VP(10));

    skiplist->funs.delete_table(skiplist, NULL, NULL);

    return (el1 == 1) && (el2 == 0);
}

int test_insert_write_over(KVSet * (*create_kvset_fun)()){
    KVSet* skiplist = create_kvset_fun();

    void * el = skiplist->funs.put(skiplist, TO_VP(10));

    skiplist->funs.put(skiplist, TO_VP(5));

    skiplist->funs.put(skiplist, TO_VP(15));

    el = skiplist->funs.put(skiplist, TO_VP(10));

    skiplist->funs.delete_table(skiplist, NULL, NULL);

    return el == TO_VP(10);
}

int test_lookup(KVSet * (*create_kvset_fun)()){
    KVSet* skiplist = create_kvset_fun();

    void * el;

    skiplist->funs.put(skiplist, TO_VP(42));

    el = skiplist->funs.lookup(skiplist, TO_VP(42));

    skiplist->funs.delete_table(skiplist, NULL, NULL);

    return el == TO_VP(42);
}

int test_lookup_not_exsisting(KVSet * (*create_kvset_fun)()){
    KVSet* skiplist = create_kvset_fun();

    void * el;

    skiplist->funs.put(skiplist, TO_VP(42));

    el = skiplist->funs.lookup(skiplist, TO_VP(43));

    skiplist->funs.delete_table(skiplist, NULL, NULL);
    
    return el == NULL;
}

int test_remove(KVSet * (*create_kvset_fun)()){
    KVSet* skiplist = create_kvset_fun();

    void * el;

    skiplist->funs.put(skiplist, TO_VP(42));

    el = skiplist->funs.lookup(skiplist, TO_VP(42));

    assert(el == TO_VP(42));

    skiplist->funs.remove(skiplist, TO_VP(42));

    el = skiplist->funs.lookup(skiplist, TO_VP(42));

    skiplist->funs.delete_table(skiplist, NULL, NULL);
    
    return el == NULL;
}


int test_insert_lookup_delete_lookup_many(KVSet * (*create_kvset_fun)()){

    int i;

    int nr_of_ops_of_each_type = 1024;

    KVSet* skiplist = create_kvset_fun();

    void * elements[nr_of_ops_of_each_type];

    
    srand((unsigned)time(0));

    //Create elements
    for(i = 0; i < nr_of_ops_of_each_type; i++){
        elements[i] = TO_VP(0xFFFFFFFFFFFFFFFF & rand());
    }

    //Inserts
    for(i = 0; i < nr_of_ops_of_each_type; i++){
        skiplist->funs.put(skiplist, elements[i]);
    }

    //Inserts again!
    for(i = 0; i < nr_of_ops_of_each_type; i++){
        assert(TO_VP(elements[i]) == skiplist->funs.put(skiplist, elements[i]));
    }

    //print_skiplist(skiplist);

    //Lookups
    for(i = 0; i < nr_of_ops_of_each_type; i++){
        assert(TO_VP(elements[i]) == skiplist->funs.lookup(skiplist, TO_VP(elements[i])));
    }

    //Deletes
    for(i = 0; i < nr_of_ops_of_each_type; i++){
        assert(TO_VP(elements[i]) == skiplist->funs.remove(skiplist, TO_VP(elements[i])));
    }

    //Lookup_again
    for(i = 0; i < nr_of_ops_of_each_type; i++){
        assert(NULL == skiplist->funs.lookup(skiplist, TO_VP(elements[i])));
    }

    skiplist->funs.delete_table(skiplist, NULL, NULL);
    
    return 1;

}

int test_first(KVSet * (*create_kvset_fun)()){
    
    KVSet* skiplist = create_kvset_fun();
    void * el;
  
    assert(skiplist->funs.first(skiplist) == NULL);

    skiplist->funs.put(skiplist, TO_VP(5));

    skiplist->funs.put(skiplist, TO_VP(3));

    skiplist->funs.put(skiplist, TO_VP(6));

    el = skiplist->funs.first(skiplist);
    
    skiplist->funs.delete_table(skiplist, NULL, NULL);
    
    return el == TO_VP(3);
}

int test_last(KVSet * (*create_kvset_fun)()){
    
    KVSet* skiplist = create_kvset_fun();
    void * el;
  
    assert(skiplist->funs.last(skiplist) == NULL);

    skiplist->funs.put(skiplist, TO_VP(3));

    skiplist->funs.put(skiplist, TO_VP(6));

    skiplist->funs.put(skiplist, TO_VP(4));

    el = skiplist->funs.last(skiplist);
    
    skiplist->funs.delete_table(skiplist, NULL, NULL);
    
    return el == TO_VP(6);
}



int test_next(KVSet * (*create_kvset_fun)()){
    KVSet* skiplist = create_kvset_fun();

    void * el;

    skiplist->funs.put(skiplist, TO_VP(3));

    skiplist->funs.put(skiplist, TO_VP(6));

    skiplist->funs.put(skiplist, TO_VP(4));

    el = skiplist->funs.first(skiplist);

    assert(el == TO_VP(3));

    el = skiplist->funs.next(skiplist, el);

    assert(el == TO_VP(4));

    el = skiplist->funs.next(skiplist, el);

    assert(el == TO_VP(6));

    el = skiplist->funs.next(skiplist, el);

    skiplist->funs.delete_table(skiplist, NULL, NULL);
    
    return el == NULL;
}



int test_previous(KVSet * (*create_kvset_fun)()){
    KVSet* skiplist = create_kvset_fun();

    void * el;

    skiplist->funs.put(skiplist, TO_VP(3));

    skiplist->funs.put(skiplist, TO_VP(6));

    skiplist->funs.put(skiplist, TO_VP(4));

    el = skiplist->funs.last(skiplist);

    assert(el == TO_VP(6));

    el = skiplist->funs.previous(skiplist, el);

    assert(el == TO_VP(4));

    el = skiplist->funs.previous(skiplist, el);

    assert(el == TO_VP(3));

    el = skiplist->funs.previous(skiplist, el);

    skiplist->funs.delete_table(skiplist, NULL, NULL);
    
    return el == NULL;

}

int number_of_accesses_per_thread = 100000;
int key_range = 10;
int remove_insert_procentage_insert = 0.7;

KVSet* global_skiplist;

void * test_concurrent_insert_thread(void * x){
    int i;
    long key;
    unsigned short * xsubi = (unsigned short *)x;
    for(i = 0; i < number_of_accesses_per_thread; i++){
        key = 1+key_range*2 + jrand48(xsubi)%key_range;
        if(key!=0)
            global_skiplist->funs.put(global_skiplist,TO_VP(key));
        //printf("%d\n",i);
    }
    return NULL;
}

int test_concurrent_insert(KVSet * (*create_kvset_fun)(),
                           int my_number_of_accesses_per_thread,
                           int my_key_range){
    int i;
    unsigned short xsubis[64*NUMBER_OF_THREADS]; 
    unsigned short * xsubiTmp;
    unsigned short * xsubi;
    pthread_t threads[NUMBER_OF_THREADS];
    number_of_accesses_per_thread = my_number_of_accesses_per_thread;
    key_range = my_key_range;
    global_skiplist = create_kvset_fun();
    for(i = 0; i < NUMBER_OF_THREADS; i++){
        xsubi = &xsubis[64*i];
        srand48(i);
        xsubiTmp = seed48(xsubi);
        xsubi[0] = xsubiTmp[0];
        xsubi[1] = xsubiTmp[1];
        xsubi[2] = xsubiTmp[2];
        pthread_create(&threads[i],NULL,&test_concurrent_insert_thread,xsubi);
    }
    for(i = 0; i < NUMBER_OF_THREADS; i++){
        pthread_join(threads[i], NULL);
    }
    global_skiplist->funs.delete_table(global_skiplist, NULL, NULL);
    return 1;
}


void * test_concurrent_insert_remove_thread(void * x){
    int i;
    long key;
    unsigned short * xsubi = (unsigned short *)x;
    for(i = 0; i < number_of_accesses_per_thread; i++){
        key = 1+key_range*2 + jrand48(xsubi)%key_range;
        if(erand48(xsubi) < remove_insert_procentage_insert){
            global_skiplist->funs.put(global_skiplist,TO_VP(key));
        }else{
            global_skiplist->funs.remove(global_skiplist,TO_VP(key));
        }
    }
    return NULL;
}



int test_concurrent_insert_remove(KVSet * (*create_kvset_fun)(),
                                  int my_number_of_accesses_per_thread,
                                  int my_key_range,
                                  int my_remove_insert_procentage_insert){
    int i;
    unsigned short xsubis[64*NUMBER_OF_THREADS]; 
    unsigned short * xsubiTmp;
    unsigned short * xsubi;
    pthread_t threads[NUMBER_OF_THREADS];
    number_of_accesses_per_thread = my_number_of_accesses_per_thread;
    key_range = my_key_range;
    remove_insert_procentage_insert = my_remove_insert_procentage_insert;
    global_skiplist = create_kvset_fun();
    for(i = 0; i < NUMBER_OF_THREADS; i++){
        xsubi = &xsubis[64*i];
        srand48(i);
        xsubiTmp = seed48(xsubi);
        xsubi[0] = xsubiTmp[0];
        xsubi[1] = xsubiTmp[1];
        xsubi[2] = xsubiTmp[2];
        pthread_create(&threads[i],NULL,&test_concurrent_insert_remove_thread,xsubi);
    }
    for(i = 0; i < NUMBER_OF_THREADS; i++){
        pthread_join(threads[i], NULL);
    }
    global_skiplist->funs.delete_table(global_skiplist, NULL, NULL);
    return 1;
}

int test_concurrent_insert_remove_lookup(KVSet * (*create_kvset_fun)()){
    KVSet* skiplist = create_kvset_fun();
    skiplist->funs.delete_table(skiplist, NULL, NULL);
    return 1;
}



/*
 * ================
 * Public interface
 * ================
 */
void test_general_kvset_properties(KVSet * (*create_kvset_fun)()){
    kvset_init();
    printf("\033[32m -- STARTING GENERAL PROPERTIES TESTS! -- \033[m\n");

    T(test_create_and_delete(create_kvset_fun), "test_create_and_delete()");
    T(test_insert(create_kvset_fun), "test_insert()");
    T(test_insert_write_over(create_kvset_fun), "test_insert_write_over()");
    T(test_lookup(create_kvset_fun), "test_lookup()");
    T(test_lookup_not_exsisting(create_kvset_fun), "test_lookup_not_exsisting()");
    T(test_remove(create_kvset_fun), "test_remove()");
    T(test_insert_lookup_delete_lookup_many(create_kvset_fun), "test_insert_lookup_delete_lookup_many()");
    //T(test_insert_new(create_kvset_fun), "test_insert_new()");

    printf("\033[32m -- GENERAL PROPERTIES TESTS COMPLETED! -- \033[m\n");  

}

void test_ordered_kvset_properties(KVSet * (*create_kvset_fun)()){
    printf("\033[32m -- STARTING ORDERED PROPERTIES TESTS! -- \033[m\n");

    T(test_first(create_kvset_fun), "test_first()");
    T(test_first(create_kvset_fun), "test_last()");
    T(test_next(create_kvset_fun), "test_next()");
    T(test_previous(create_kvset_fun), "test_previous()");

    printf("\033[32m -- ORDERED PROPERTIES TESTS COMPLETED! -- \033[m\n");
}


void test_concurrent_kvset_properties(KVSet * (*create_kvset_fun)()){
    printf("\033[32m -- STARTING CONCURRENT PROPERTIES TESTS! -- \033[m\n");
    kvset_init();
    T(test_concurrent_insert(create_kvset_fun, 10000, 1), "test_concurrent_insert(10000, 1)");
    T(test_concurrent_insert(create_kvset_fun, 10000, 2), "test_concurrent_insert(10000, 2)");
    T(test_concurrent_insert(create_kvset_fun, 10000, 4), "test_concurrent_insert(10000, 4)");
    T(test_concurrent_insert(create_kvset_fun, 100000, 8), "test_concurrent_insert(100000, 8)");
    T(test_concurrent_insert(create_kvset_fun, 100000, 16), "test_concurrent_insert(100000, 16)");
    T(test_concurrent_insert(create_kvset_fun, 100000, 32), "test_concurrent_insert(100000, 32)");
    T(test_concurrent_insert(create_kvset_fun, 1000000, 1000), "test_concurrent_insert(1000000, 1000)");
    T(test_concurrent_insert(create_kvset_fun, 1000000, 10000), "test_concurrent_insert(1000000, 10000)");
    T(test_concurrent_insert(create_kvset_fun, 1000000, 100000), "test_concurrent_insert(1000000, 100000)");
    T(test_concurrent_insert(create_kvset_fun, 1000000, 1000000), "test_concurrent_insert(1000000, 1000000)");
    T(test_concurrent_insert(create_kvset_fun, 1000000, 10000000), "test_concurrent_insert(1000000, 10000000)");
    
    T(test_concurrent_insert_remove(create_kvset_fun, 10000, 1, 0.7), "test_concurrent_insert_remove(10000, 1, 0.7)");
    T(test_concurrent_insert_remove(create_kvset_fun, 10000, 2, 0.7), "test_concurrent_insert_remove(10000, 2, 0.7)");
    T(test_concurrent_insert_remove(create_kvset_fun, 10000, 4, 0.7), "test_concurrent_insert_remove(10000, 4, 0.7)");
    T(test_concurrent_insert_remove(create_kvset_fun, 100000, 8, 0.7), "test_concurrent_insert_remove(100000, 8, 0.7)");
    T(test_concurrent_insert_remove(create_kvset_fun, 100000, 16, 0.7), "test_concurrent_insert_remove(100000, 16, 0.7)");
    T(test_concurrent_insert_remove(create_kvset_fun, 100000, 32, 0.7), "test_concurrent_insert_remove(100000, 32, 0.7)");
    T(test_concurrent_insert_remove(create_kvset_fun, 1000000, 1000, 0.7), "test_concurrent_insert_remove(1000000, 1000, 0.7)");
    T(test_concurrent_insert_remove(create_kvset_fun, 1000000, 10000, 0.7), "test_concurrent_insert_remove(1000000, 10000, 0.7)");
    T(test_concurrent_insert_remove(create_kvset_fun, 1000000, 100000, 0.7), "test_concurrent_insert_remove(1000000, 100000, 0.7)");
    T(test_concurrent_insert_remove(create_kvset_fun, 1000000, 1000000, 0.7), "test_concurrent_insert_remove(1000000, 1000000, 0.7)");
    T(test_concurrent_insert_remove(create_kvset_fun, 1000000, 10000000, 0.7), "test_concurrent_insert_remove(1000000, 10000000, 0.7)");

    //T(test_concurrent_insert_remove_lookup(create_kvset_fun), "test_concurrent_insert_remove_lookup()");

    printf("\033[32m -- CONCURRENT PROPERTIES TESTS COMPLETED! -- \033[m\n");
}

void test_unordered_kvset_properties(KVSet * (*create_kvset_fun)()){


}


