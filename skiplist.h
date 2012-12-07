#ifndef __SKIPLIST_H__
#define __SKIPLIST_H__

#include "kvset.h"
#include "stdlib.h"

KVSet * new_skiplist(int (*compare_function)(void *, void *, int), 
                     void (*free_function)(void *),
                     void *(*malloc_function)(size_t),
                     int key_pos); 

KVSet * new_skiplist_default();

#endif
