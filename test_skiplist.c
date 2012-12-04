
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "assert.h"


#include "skiplist.h"

#define TO_VP(intValue) (void *)(intValue)

#define T(testFunCall, tesName)                 \
    printf("STARTING TEST: ");                  \
    test(testFunCall, tesName);

void test(int success, char msg[]){

    if(success){
        printf("\033[32m -- SUCCESS! -- \033[m");
    }else{
        printf("\033[31m -- FAIL! -- \033[m");
    }

    printf("TEST: %s\n", msg);

}

/*
void print_skiplist(SkiplistNode* skiplist){

    int i = 0;
    int n;
    printf("PRINTING SKIPLIST\n===============================================\n");

    while(!(skiplist->info & SKIPLIST_RIGHT_BORDER_NODE)){
        printf("NODE NR: %d      ref %lu\n", i, skiplist);
        printf("VALUE %d\n",skiplist->element);
        printf("NUM LEVELS %d\n",skiplist->num_of_levels);
        for(n = 0; n < skiplist->num_of_levels; n++){
            printf("SUB LIST %d: %lu\n",n,skiplist->lower_lists[n]);
            printf("__________________________________________________________________\n");
        }
        i++;
        skiplist = skiplist->lower_lists[skiplist->num_of_levels-1];
    
    }

    printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");

}
*/

int test_create_and_delete(){
    KVSet* skiplist = new_skiplist_default();
    skiplist->funs.delete(skiplist, NULL);
    return 1;
}

int test_insert(){
    KVSet* skiplist = new_skiplist_default();

    void * el = skiplist->funs.put(skiplist, TO_VP(10));

    skiplist->funs.delete(skiplist, NULL);

    return el == NULL;
}

int test_insert_new(){
    KVSet* skiplist = new_skiplist_default();
              
    int el1 = skiplist->funs.put_new(skiplist, TO_VP(10));

    assert(skiplist->funs.lookup(skiplist, TO_VP(10)) == TO_VP(10));

    int el2 = skiplist->funs.put_new(skiplist, TO_VP(10));

    skiplist->funs.delete(skiplist, NULL);

    return (el1 == 1) && (el2 == 0);
}

int test_insert_write_over(){
    KVSet* skiplist = new_skiplist_default();

    void * el = skiplist->funs.put(skiplist, TO_VP(10));

    skiplist->funs.put(skiplist, TO_VP(5));

    skiplist->funs.put(skiplist, TO_VP(15));

    el = skiplist->funs.put(skiplist, TO_VP(10));

    skiplist->funs.delete(skiplist, NULL);

    return el == TO_VP(10);
}

int test_lookup(){
    KVSet* skiplist = new_skiplist_default();

    void * el;

    skiplist->funs.put(skiplist, TO_VP(42));

    el = skiplist->funs.lookup(skiplist, TO_VP(42));

    skiplist->funs.delete(skiplist, NULL);

    return el == TO_VP(42);
}

int test_lookup_not_exsisting(){
    KVSet* skiplist = new_skiplist_default();

    void * el;

    skiplist->funs.put(skiplist, TO_VP(42));

    el = skiplist->funs.lookup(skiplist, TO_VP(43));

    skiplist->funs.delete(skiplist, NULL);
    
    return el == NULL;
}

int test_remove(){
    KVSet* skiplist = new_skiplist_default();

    void * el;

    skiplist->funs.put(skiplist, TO_VP(42));

    el = skiplist->funs.lookup(skiplist, TO_VP(42));

    assert(el == TO_VP(42));

    skiplist->funs.remove(skiplist, TO_VP(42));

    el = skiplist->funs.lookup(skiplist, TO_VP(42));

    skiplist->funs.delete(skiplist, NULL);
    
    return el == NULL;
}


int test_insert_lookup_delete_lookup_many(){

    int i;

    int nr_of_ops_of_each_type = 1024;

    KVSet* skiplist = new_skiplist_default();

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

    skiplist->funs.delete(skiplist, NULL);
    
    return 1;

}

int test_first(){
    
    KVSet* skiplist = new_skiplist_default();
    void * el;
  
    assert(skiplist->funs.first(skiplist) == NULL);

    skiplist->funs.put(skiplist, TO_VP(5));

    skiplist->funs.put(skiplist, TO_VP(3));

    skiplist->funs.put(skiplist, TO_VP(6));

    el = skiplist->funs.first(skiplist);
    
    skiplist->funs.delete(skiplist, NULL);
    
    return el == TO_VP(3);
}

int test_last(){
    
    KVSet* skiplist = new_skiplist_default();
    void * el;
  
    assert(skiplist->funs.last(skiplist) == NULL);

    skiplist->funs.put(skiplist, TO_VP(3));

    skiplist->funs.put(skiplist, TO_VP(6));

    skiplist->funs.put(skiplist, TO_VP(4));

    el = skiplist->funs.last(skiplist);
    
    skiplist->funs.delete(skiplist, NULL);
    
    return el == TO_VP(6);
}



int test_next(){
    KVSet* skiplist = new_skiplist_default();

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

    skiplist->funs.delete(skiplist, NULL);
    
    return el == NULL;
}



int test_previous(){
    KVSet* skiplist = new_skiplist_default();

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

    skiplist->funs.delete(skiplist, NULL);
    
    return el == NULL;

}

int main(int argc, char **argv){

    printf("\n\n\033[32m -- STARTING TESTS! -- \033[m\n\n");

    T(test_create_and_delete(), "test_create_and_delete()");
    T(test_insert(), "test_insert()");   
    T(test_insert_write_over(), "test_insert_write_over()");
    T(test_lookup(), "test_lookup()");
    T(test_lookup_not_exsisting(), "test_lookup_not_exsisting()");
    T(test_remove(), "test_remove()");
    T(test_insert_lookup_delete_lookup_many(), "test_insert_lookup_delete_lookup_many()");
    T(test_insert_new(), "test_insert_new()");
    T(test_first(), "test_first()");
    T(test_first(), "test_last()");
    T(test_next(), "test_next()");
    T(test_previous(), "test_previous()");

    printf("\n\n\033[32m -- TESTS COMPLETED! -- \033[m\n\n");

    exit(0);

}

