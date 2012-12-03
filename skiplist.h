
#include "limits.h"
/*
 * Defintion of the element type
 */

typedef void * SkiplistElement;

#define SKIPLIST_NULL_ELEMENT NULL

#define SKIPLIST_CMP_ELEMENTS(E1, E2) (E1 > E2 ? 1 : (E1 < E2 ? -1 : 0))

/*
 * Skip list data structure
 */

#define SKIPLIST_NUM_OF_LEVELS 30

#define SKIPLIST_NORMAL_NODE 1
#define SKIPLIST_LEFT_BORDER_NODE 1 << 2
#define SKIPLIST_RIGHT_BORDER_NODE 1 << 1


typedef struct skiplist {
    //contains information about if it is a boarder point
    int info;
    SkiplistElement  element;
    int  num_of_levels;
    struct skiplist * lower_lists[];    
} SkipList;


/*
 * Public interface
 */
SkipList * skiplist_new();

void skiplist_delete(SkipList* skiplist);

int skiplist_put_new(SkipList* skiplist, SkiplistElement element);

SkiplistElement skiplist_put(SkipList* skiplist, SkiplistElement element);

SkiplistElement skiplist_remove(SkipList* skiplist, SkiplistElement element);

SkiplistElement skiplist_lookup(SkipList* skiplist, SkiplistElement element);

SkiplistElement skiplist_first(SkipList* skiplist);

SkiplistElement skiplist_last(SkipList* skiplist);

SkiplistElement skiplist_next(SkipList* skiplist, SkiplistElement element);

SkiplistElement skiplist_previous(SkipList* skiplist, SkiplistElement element);



