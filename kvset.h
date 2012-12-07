#ifndef __KVSET_H__
#define __KVSET_H__

typedef struct kv_set KVSet;

typedef struct kv_set_functions
{   
    void (*delete_table)(KVSet * set, void (*element_free_function)(void *));
    void * (*put)(KVSet * set, void * key);
    int (*put_new)(KVSet * set, void * key);
    void * (*remove)(KVSet * set, void * key);
    void * (*lookup)(KVSet * set, void * key);
    int (*member)(KVSet * set, void * key);
    void * (*first)(KVSet * set);
    void * (*last)(KVSet * set);    
    void * (*next)(KVSet * set, void * key);
    void * (*previous)(KVSet * set, void * key);
} KVSetFunctions;


typedef struct kv_set
{
    KVSetFunctions funs;
    void * type_specific_data;
} KVSet;

#endif
