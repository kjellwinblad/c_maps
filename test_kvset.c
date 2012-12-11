#include "test_kvset.h"

#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "assert.h"


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

    void * el = skiplist->funs.put(skiplist, TO_VP(10), 0);

    skiplist->funs.delete_table(skiplist, NULL, NULL);

    return el == NULL;
}

int test_insert_new(KVSet * (*create_kvset_fun)()){
    KVSet* skiplist = create_kvset_fun();
              
    int el1 = skiplist->funs.put_new(skiplist, TO_VP(10), 0);

    assert(skiplist->funs.lookup(skiplist, TO_VP(10)) == TO_VP(10));

    int el2 = skiplist->funs.put_new(skiplist, TO_VP(10), 0);

    skiplist->funs.delete_table(skiplist, NULL, NULL);

    return (el1 == 1) && (el2 == 0);
}

int test_insert_write_over(KVSet * (*create_kvset_fun)()){
    KVSet* skiplist = create_kvset_fun();

    void * el = skiplist->funs.put(skiplist, TO_VP(10), 0);

    skiplist->funs.put(skiplist, TO_VP(5), 0);

    skiplist->funs.put(skiplist, TO_VP(15), 0);

    el = skiplist->funs.put(skiplist, TO_VP(10), 0);

    skiplist->funs.delete_table(skiplist, NULL, NULL);

    return el == TO_VP(10);
}

int test_lookup(KVSet * (*create_kvset_fun)()){
    KVSet* skiplist = create_kvset_fun();

    void * el;

    skiplist->funs.put(skiplist, TO_VP(42), 0);

    el = skiplist->funs.lookup(skiplist, TO_VP(42));

    skiplist->funs.delete_table(skiplist, NULL, NULL);

    return el == TO_VP(42);
}

int test_lookup_not_exsisting(KVSet * (*create_kvset_fun)()){
    KVSet* skiplist = create_kvset_fun();

    void * el;

    skiplist->funs.put(skiplist, TO_VP(42), 0);

    el = skiplist->funs.lookup(skiplist, TO_VP(43));

    skiplist->funs.delete_table(skiplist, NULL, NULL);
    
    return el == NULL;
}

int test_remove(KVSet * (*create_kvset_fun)()){
    KVSet* skiplist = create_kvset_fun();

    void * el;

    skiplist->funs.put(skiplist, TO_VP(42), 0);

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
        skiplist->funs.put(skiplist, elements[i], 0);
    }

    //Inserts again!
    for(i = 0; i < nr_of_ops_of_each_type; i++){
        assert(TO_VP(elements[i]) == skiplist->funs.put(skiplist, elements[i], 0));
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

    skiplist->funs.put(skiplist, TO_VP(5), 0);

    skiplist->funs.put(skiplist, TO_VP(3), 0);

    skiplist->funs.put(skiplist, TO_VP(6), 0);

    el = skiplist->funs.first(skiplist);
    
    skiplist->funs.delete_table(skiplist, NULL, NULL);
    
    return el == TO_VP(3);
}

int test_last(KVSet * (*create_kvset_fun)()){
    
    KVSet* skiplist = create_kvset_fun();
    void * el;
  
    assert(skiplist->funs.last(skiplist) == NULL);

    skiplist->funs.put(skiplist, TO_VP(3), 0);

    skiplist->funs.put(skiplist, TO_VP(6), 0);

    skiplist->funs.put(skiplist, TO_VP(4), 0);

    el = skiplist->funs.last(skiplist);
    
    skiplist->funs.delete_table(skiplist, NULL, NULL);
    
    return el == TO_VP(6);
}



int test_next(KVSet * (*create_kvset_fun)()){
    KVSet* skiplist = create_kvset_fun();

    void * el;

    skiplist->funs.put(skiplist, TO_VP(3), 0);

    skiplist->funs.put(skiplist, TO_VP(6), 0);

    skiplist->funs.put(skiplist, TO_VP(4), 0);

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

    skiplist->funs.put(skiplist, TO_VP(3), 0);

    skiplist->funs.put(skiplist, TO_VP(6), 0);

    skiplist->funs.put(skiplist, TO_VP(4), 0);

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

/*
 * ================
 * Public interface
 * ================
 */
void test_general_kvset_properties(KVSet * (*create_kvset_fun)()){

    printf("\033[32m -- STARTING GENERAL PROPERTIES TESTS! -- \033[m\n");

    T(test_create_and_delete(create_kvset_fun), "test_create_and_delete()");
    T(test_insert(create_kvset_fun), "test_insert()");   
    T(test_insert_write_over(create_kvset_fun), "test_insert_write_over()");
    T(test_lookup(create_kvset_fun), "test_lookup()");
    T(test_lookup_not_exsisting(create_kvset_fun), "test_lookup_not_exsisting()");
    T(test_remove(create_kvset_fun), "test_remove()");
    T(test_insert_lookup_delete_lookup_many(create_kvset_fun), "test_insert_lookup_delete_lookup_many()");
    T(test_insert_new(create_kvset_fun), "test_insert_new()");

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

void test_unordered_kvset_properties(KVSet * (*create_kvset_fun)()){


}


