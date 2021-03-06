#include "concurrent_skiplist.h"
#include "stdio.h"
#include "pthread.h"
#include "smp_utils.h"
#include "assert.h"
#include "hazard_pointers.h"
/*
 * ========================
 * Internal data structures
 * ========================
 */


#define SKIPLIST_NUM_OF_LEVELS 30

#define SKIPLIST_NORMAL_NODE 1
#define SKIPLIST_LEFT_BORDER_NODE 1 << 2
#define SKIPLIST_RIGHT_BORDER_NODE 1 << 1

pthread_key_t random_seed_key;


typedef struct skiplist_node {
    //contains information about if it is a boarder point
    int info;
    int  num_of_levels;
    void *  element;
    int marked;
    int fully_linked;
    pthread_mutex_t lock;
    struct skiplist_node * lower_lists[];    
} SkiplistNode;

typedef struct skiplist {
    HazardPointerData hazard_pointer_data;
    int (*compare)(void *, void *);
    void (*free)(void *);
    void *(*malloc)(size_t size);
    SkiplistNode head_node;
    SkiplistNode * end_node;
} Skiplist;

struct one_level_find_result {
    SkiplistNode * neigbour_before;
    SkiplistNode * element_skiplist;
    SkiplistNode * neigbour_after;
};

struct find_result {
    int level_found;
    SkiplistNode * element_skiplist;
    SkiplistNode * neigbours_before[SKIPLIST_NUM_OF_LEVELS];
    SkiplistNode * neigbours_after[SKIPLIST_NUM_OF_LEVELS];    
};


struct validate_lock_result {
    int highest_locked;
    int valid;
    int lock_unlock_count;
    pthread_mutex_t * locks_to_unlock[SKIPLIST_NUM_OF_LEVELS];
};

/*
 * ==================
 * Internal functions
 * ==================
 */

static inline 
SkiplistNode* create_skiplist_node(int num_of_levels, 
                                   void * element,
                                   void *(*malloc_fun)(size_t)){
    SkiplistNode* skiplist = 
        (SkiplistNode*)malloc_fun(sizeof(SkiplistNode) + 
                                  sizeof(SkiplistNode*) * (num_of_levels));
    skiplist->element = element;
    skiplist->num_of_levels = num_of_levels;
    skiplist->info = SKIPLIST_NORMAL_NODE;
    pthread_mutex_init (&skiplist->lock, NULL);
    skiplist->marked = 0;
    skiplist->fully_linked = 0;
    return skiplist;
}

static
unsigned short * init_random_seed_thread(){
    unsigned short * seed =  malloc(sizeof(unsigned short) * 3);
    unsigned short * seedResult;
    srand48(pthread_self());
    seedResult = seed48(seed);
    seed[0] = seedResult[0];
    seed[1] = seedResult[1];
    seed[2] = seedResult[2];
    pthread_setspecific(random_seed_key, seed);
    return seed;
}

static inline 
int random_level(int num_of_levels){
    int i;
    long random_number;
    int num;
    unsigned short * seed = pthread_getspecific(random_seed_key);
    if(seed == NULL){
        seed = init_random_seed_thread();
    }
    random_number = 2147483648 + jrand48(seed);
    num = 2;
    for(i = num_of_levels -1 ; i > 0 ; i--){
        if(random_number > (2147483648 / num)){
            return i;
        }
        num = num * 2;
    }
    return 0;
}

static inline 
int compare(SkiplistNode* skiplist,
            void * key, 
            int (*compare_function)(void *, void *), 
            unsigned int key_offset){
    if(skiplist->info & SKIPLIST_NORMAL_NODE){
        return compare_function(skiplist->element + key_offset, key);
    } else if (skiplist->info & SKIPLIST_LEFT_BORDER_NODE){
        return -1;
    } else {
        return 1;
    }
}

static inline
void remove_hazard_pointers(HazardPointerData * hpdata, struct find_result * neigbours){
    hazard_pointer_remove_all(hpdata, 63);
}

static inline 
struct one_level_find_result find_neigbours_1_level(SkiplistNode* skiplist,
                                                    void * element,
                                                    int level,
                                                    int (*compare_function)(void *, void *),
                                                    unsigned int key_offset,
                                                    HazardPointerData * hpdata){
    int cmp_result;
    SkiplistNode* skiplist_prev = skiplist;
    int level_pos = level - (SKIPLIST_NUM_OF_LEVELS - skiplist_prev->num_of_levels);
    SkiplistNode* skiplist_next;
    struct one_level_find_result result;
    int prev_hp_slot = level;
    int next_hp_slot = level + 1;
    int element_skiplist_pos = 61;
    skiplist_next = hazard_pointer_set_check(hpdata, next_hp_slot,
                                             (void**)&skiplist_prev->lower_lists[level_pos], next_hp_slot - 2);
    do{
        cmp_result = compare(skiplist_next,
                             element,
                             compare_function,
                             key_offset);
        if(0 < cmp_result){
            result.neigbour_before = skiplist_prev;
            result.element_skiplist = NULL;
            result.neigbour_after = skiplist_next;
        } else if(0 == cmp_result){
            level_pos = level - (SKIPLIST_NUM_OF_LEVELS - skiplist_next->num_of_levels);
            result.neigbour_before = skiplist_prev;
            result.element_skiplist = skiplist_next;
            result.neigbour_after = 
                hazard_pointer_move_set_check(hpdata, element_skiplist_pos, next_hp_slot,
                                        (void**)&skiplist_next->lower_lists[level_pos], next_hp_slot - 2);
        } else {
            level_pos = level - (SKIPLIST_NUM_OF_LEVELS - skiplist_next->num_of_levels);
            skiplist_prev = skiplist_next;
            skiplist_next = 
                hazard_pointer_move_set_check(hpdata, prev_hp_slot, next_hp_slot,
                                              (void**)&skiplist_next->lower_lists[level_pos], next_hp_slot-2);
        }
    } while(0 > cmp_result);

    return result;

}

static inline 
struct find_result find_neigbours(SkiplistNode* skiplist,
                                  void * element,
                                  int (*compare_function)(void *, void *),
                                  unsigned int key_offset,
                                  HazardPointerData * hpdata){

    int level;
    struct find_result result;
    struct one_level_find_result level_result;
    SkiplistNode* neigbour_before_iter = skiplist;
    int level_found = -1;
    level_result.element_skiplist = NULL; 
    for(level = 0; level < SKIPLIST_NUM_OF_LEVELS; level++){
        level_result = 
            find_neigbours_1_level(neigbour_before_iter, 
                                   element, 
                                   level, 
                                   compare_function, 
                                   key_offset,
                                   hpdata);
        result.neigbours_before[level] = level_result.neigbour_before;
        result.neigbours_after[level] = level_result.neigbour_after;
        neigbour_before_iter = level_result.neigbour_before;
        if(level_result.element_skiplist != NULL){
            if(level_found == -1){
                level_found = level;
            }
        }
    }
    result.level_found = level_found;
    result.element_skiplist = level_result.element_skiplist;
    return result;
}

static inline
void set_next_at_level(SkiplistNode* skiplist,
                       SkiplistNode* next_skiplist,
                       int level){
    int level_pos = level - (SKIPLIST_NUM_OF_LEVELS - skiplist->num_of_levels);
    skiplist->lower_lists[level_pos] = next_skiplist;
}


static inline 
struct validate_lock_result validate_and_lock(struct find_result * neigbours, 
                                              int level_to_insert_to,
                                              HazardPointerData * hpdata){
    int link_level;
    int valid = 1;
    SkiplistNode* pred;
    SkiplistNode* last_pred = NULL;
    SkiplistNode* succs;
    struct validate_lock_result result;
    int highest_locked = SKIPLIST_NUM_OF_LEVELS - 1;
    int level_pos;
    result.lock_unlock_count = 0;
    for(link_level = SKIPLIST_NUM_OF_LEVELS - 1; 
        valid && link_level >= level_to_insert_to; 
        link_level--){
        pred = neigbours->neigbours_before[link_level];

        if(neigbours->element_skiplist != NULL){
            level_pos = link_level - (SKIPLIST_NUM_OF_LEVELS - neigbours->element_skiplist->num_of_levels);
            succs = 
                hazard_pointer_set(hpdata, link_level + 1, 
                                   (void**)&neigbours->element_skiplist->lower_lists[level_pos]);
            neigbours->neigbours_after[link_level] = succs;
        }else{
            succs = neigbours->neigbours_after[link_level];            
        }
 
        if(pred != last_pred){//Otherwise already locked
            result.locks_to_unlock[result.lock_unlock_count] = &pred->lock;
            result.lock_unlock_count++;
            pthread_mutex_lock(&pred->lock);
            last_pred = pred;
        }
        highest_locked = link_level;
        level_pos = link_level - (SKIPLIST_NUM_OF_LEVELS - pred->num_of_levels);

        valid = !ACCESS_ONCE(pred->marked) && 
            ACCESS_ONCE(succs->fully_linked) &&
            (ACCESS_ONCE(pred->lower_lists[level_pos]) == 
             (neigbours->element_skiplist == NULL ? succs : neigbours->element_skiplist));

    }
    result.highest_locked = highest_locked;
    result.valid = valid;

    return result;
}

static inline 
void unlock_to_level(struct validate_lock_result * to_clean_up){
    int i;
    for(i = 0; i < to_clean_up->lock_unlock_count; i++){
        pthread_mutex_unlock(to_clean_up->locks_to_unlock[i]);
    }
}

static inline 
int try_insert_sublist(SkiplistNode* skiplist, 
                    struct find_result * neigbours, 
                    SkiplistNode* sublist, 
                    int level_to_insert_to){
    int link_level;
    struct validate_lock_result validate_result = 
        validate_and_lock(neigbours, level_to_insert_to, NULL);

    if(validate_result.valid){
        for(link_level = SKIPLIST_NUM_OF_LEVELS - 1; 
            link_level >= level_to_insert_to; 
            link_level--){
                set_next_at_level(sublist,
                                  neigbours->neigbours_after[link_level],
                                  link_level);
        }
        __sync_synchronize();
        for(link_level = SKIPLIST_NUM_OF_LEVELS - 1; 
            link_level >= level_to_insert_to; 
            link_level--){
            set_next_at_level(neigbours->neigbours_before[link_level],
                              sublist,
                              link_level);
            
        }
        __sync_synchronize();
        sublist->fully_linked = 1;
        __sync_synchronize();
    }
    unlock_to_level(&validate_result);
    return validate_result.valid;
}


static inline 
int try_remove_element(struct find_result * neigbours, 
                       int level_to_remove_to,
                       HazardPointerData * hpdata){
    int link_level;
    int level_pos;
    struct validate_lock_result validate_result = 
        validate_and_lock(neigbours, level_to_remove_to, hpdata);
    
    if(validate_result.valid){
        for(link_level = SKIPLIST_NUM_OF_LEVELS - 1; 
            link_level >= level_to_remove_to; 
            link_level--){
            level_pos = link_level - (SKIPLIST_NUM_OF_LEVELS - neigbours->element_skiplist->num_of_levels);
            set_next_at_level(neigbours->neigbours_before[link_level],
                              //neigbours->neigbours_after[link_level],
                              neigbours->element_skiplist->lower_lists[level_pos],
                              link_level);
        }
        __sync_synchronize();
    }

    unlock_to_level(&validate_result);

    return validate_result.valid;
}

static
int default_compare_function(void * e1, void * e2){
    return (e1 > e2 ? 1 : (e1 < e2 ? -1 : 0));
}

/*
 * Internal function used in the external data structure
 */

static
void skiplist_delete(KVSet* kv_set,
                     void (*element_free_function)(void *context, void* element),
                     void * context){
    
    Skiplist * skiplist = (Skiplist *) &(kv_set->type_specific_data);
    SkiplistNode * head_node = &(skiplist->head_node);

    SkiplistNode* node_temp = head_node->lower_lists[head_node->num_of_levels -1];
    SkiplistNode* node_iter = node_temp;
    while(node_iter->info &  SKIPLIST_NORMAL_NODE){
        node_temp = node_iter;
        node_iter = node_iter->lower_lists[node_iter->num_of_levels -1];
        if(NULL != element_free_function){
            element_free_function(context, node_temp->element);
        }
        skiplist->free(node_temp);
    }
    hazard_pointer_free_everything(&skiplist->hazard_pointer_data, skiplist->free);
    skiplist->free(node_iter);
    skiplist->free(kv_set);
    return;
}

static
void * skiplist_put(KVSet* kv_set, void * key_value){

    Skiplist * skiplist = (Skiplist *) &(kv_set->type_specific_data);
    HazardPointerData * hpdata = &skiplist->hazard_pointer_data;    
    SkiplistNode * head_node = &(skiplist->head_node);
    SkiplistNode * element_skiplist;
    SkiplistNode * old_element;
    SkiplistNode* new_skiplist_node;
    void * key = key_value + kv_set->key_offset;
    int success;
    while(1){
        struct find_result neigbours = find_neigbours(head_node, 
                                                      key, 
                                                      skiplist->compare,
                                                      kv_set->key_offset,
                                                      hpdata);

        element_skiplist = neigbours.element_skiplist;
    
        if(element_skiplist == NULL){
       
            int level = random_level(head_node->num_of_levels);
            int num_of_elements_in_insert_level = head_node->num_of_levels - level;
            new_skiplist_node =
                create_skiplist_node(num_of_elements_in_insert_level, key_value, skiplist->malloc);

           success = try_insert_sublist(head_node, &neigbours, new_skiplist_node, level);
           remove_hazard_pointers(hpdata, &neigbours);
           if(success){
               return NULL;
           }else{
               skiplist->free(new_skiplist_node);
           }
        } else if(!ACCESS_ONCE(element_skiplist->marked)){
            while(!ACCESS_ONCE(element_skiplist->fully_linked)){}
            pthread_mutex_lock(&element_skiplist->lock);          
            if(!ACCESS_ONCE(element_skiplist->marked)){
                remove_hazard_pointers(hpdata, &neigbours);
                old_element = ACCESS_ONCE(element_skiplist->element);

                element_skiplist->element = key_value;
                __sync_synchronize();

                pthread_mutex_unlock(&element_skiplist->lock);
                
                return old_element;

            }else{
                remove_hazard_pointers(hpdata, &neigbours);
                pthread_mutex_unlock(&element_skiplist->lock);
            }
        }else{
            remove_hazard_pointers(hpdata, &neigbours);
            return NULL;
        }
    }
}

static
int skiplist_put_new(KVSet* kv_set, void * key_value){
    //TODO NOT THREAD SAFE!!!!
    Skiplist * skiplist = (Skiplist *) &(kv_set->type_specific_data);
    HazardPointerData * hpdata = &skiplist->hazard_pointer_data;
    SkiplistNode * head_node = &(skiplist->head_node);
    void * key = key_value + kv_set->key_offset;
    struct find_result neigbours = find_neigbours(head_node, key, skiplist->compare, kv_set->key_offset, hpdata);
    int level = random_level(head_node->num_of_levels);
    int num_of_elements_in_insert_level = head_node->num_of_levels - level;
    SkiplistNode* new_skiplist_node;
    if(neigbours.element_skiplist == NULL){
        new_skiplist_node =
            create_skiplist_node(num_of_elements_in_insert_level, key_value, skiplist->malloc);
        try_insert_sublist(head_node, &neigbours, new_skiplist_node, level);
        return 1;
    } else {
        return 0;
    }
}

static
void * skiplist_remove(KVSet* kv_set, void * key){

    Skiplist * skiplist = (Skiplist *) &(kv_set->type_specific_data);
    HazardPointerData * hpdata = &skiplist->hazard_pointer_data;
    SkiplistNode * head_node = &(skiplist->head_node);
    SkiplistNode * element_skiplist;
    struct find_result neigbours;
    int deleted_by_us = 0;
    void * element_to_return_on_success = NULL;
    int success = 0;
    while(1){
        neigbours = find_neigbours(head_node,
                                   key, skiplist->compare,
                                   kv_set->key_offset,
                                   hpdata);
        element_skiplist = neigbours.element_skiplist;

        if(element_skiplist == NULL){
            remove_hazard_pointers(hpdata, &neigbours);
            return NULL;    
        }else if(deleted_by_us){
            //Try to finalize remove again
            pthread_mutex_lock(&element_skiplist->lock);
            
            success = try_remove_element(&neigbours, 
                                         SKIPLIST_NUM_OF_LEVELS - neigbours.element_skiplist->num_of_levels,
                                         hpdata);
            pthread_mutex_unlock(&element_skiplist->lock);
            remove_hazard_pointers(hpdata, &neigbours);
            if(success){
                hazard_pointer_free(hpdata, 
                                    neigbours.element_skiplist,
                                    skiplist->free);
                return element_to_return_on_success;
            }
        }else if(ACCESS_ONCE(element_skiplist->marked)){
            remove_hazard_pointers(hpdata, &neigbours);
            return NULL;
        } else {
            //Attempt to remove
            while(!ACCESS_ONCE(element_skiplist->fully_linked)){}
            pthread_mutex_lock(&element_skiplist->lock);          
            if(!ACCESS_ONCE(element_skiplist->marked)){

                element_skiplist->marked = 1;
                  
                __sync_synchronize();

                element_to_return_on_success = ACCESS_ONCE(element_skiplist->element);

                deleted_by_us = 1;

                success = try_remove_element(&neigbours, 
                                             SKIPLIST_NUM_OF_LEVELS - neigbours.element_skiplist->num_of_levels,
                                             hpdata);
                pthread_mutex_unlock(&element_skiplist->lock);
                remove_hazard_pointers(hpdata, &neigbours);
                if(success){
                    hazard_pointer_free(hpdata, 
                                        neigbours.element_skiplist,
                                        skiplist->free);
                    return element_to_return_on_success;
                }
            }else{
                pthread_mutex_unlock(&element_skiplist->lock);
                remove_hazard_pointers(hpdata, &neigbours);
            }

        }
    }
}

static
void * skiplist_lookup(KVSet* kv_set, void * key){
    Skiplist * skiplist = (Skiplist *) &(kv_set->type_specific_data);
    HazardPointerData * hpdata = &skiplist->hazard_pointer_data;
    SkiplistNode * head_node = &(skiplist->head_node);
    void * return_value;
    struct find_result neigbours = find_neigbours(head_node, 
                                                  key, 
                                                  skiplist->compare, 
                                                  kv_set->key_offset,
                                                  hpdata);
    SkiplistNode * element_skiplist = neigbours.element_skiplist;
    if(element_skiplist != NULL &&
       ACCESS_ONCE(element_skiplist->fully_linked) &&
       !ACCESS_ONCE(element_skiplist->marked)){
        return_value = ACCESS_ONCE(element_skiplist->element);
        remove_hazard_pointers(hpdata, &neigbours);
        return return_value;
    } else {
        remove_hazard_pointers(hpdata, &neigbours);
        return NULL;
    }
}

static
int member(KVSet * kv_set, void * key){
    Skiplist * skiplist = (Skiplist *) &(kv_set->type_specific_data);
    HazardPointerData * hpdata = &skiplist->hazard_pointer_data;
    SkiplistNode * head_node = &(skiplist->head_node);

    struct find_result neigbours = find_neigbours(head_node, 
                                                  key, 
                                                  skiplist->compare, 
                                                  kv_set->key_offset,
                                                  hpdata);

    SkiplistNode * element_skiplist = neigbours.element_skiplist;

    return element_skiplist != NULL &&
        ACCESS_ONCE(element_skiplist->fully_linked) &&
        !ACCESS_ONCE(element_skiplist->marked);
}


static
void * skiplist_first(KVSet* kv_set){
    //TODO NOT THREAD SAFE!
    Skiplist * skiplist = (Skiplist *) &(kv_set->type_specific_data);
    SkiplistNode * head_node = &(skiplist->head_node);

    SkiplistNode * firstCandidate = 
        head_node->lower_lists[head_node->num_of_levels - 1];
    if(firstCandidate->info & SKIPLIST_RIGHT_BORDER_NODE){
        return NULL;
    } else {
        return firstCandidate->element;
    }
}

static
void * skiplist_last(KVSet* kv_set){
    //TODO NOT THREAD SAFE!
    Skiplist * skiplist = (Skiplist *) &(kv_set->type_specific_data);
    SkiplistNode * head_node = &(skiplist->head_node);

    int level;
    int level_pos;
    SkiplistNode* skiplist_iter_prev = head_node;
    SkiplistNode* skiplist_iter;
    
    for(level = 0; level < SKIPLIST_NUM_OF_LEVELS; level++){
        level_pos = level - (SKIPLIST_NUM_OF_LEVELS - skiplist_iter_prev->num_of_levels);
        skiplist_iter = skiplist_iter_prev->lower_lists[level_pos];
        while(skiplist_iter->info & 
              (SKIPLIST_LEFT_BORDER_NODE | SKIPLIST_NORMAL_NODE)) {
            skiplist_iter_prev = skiplist_iter;
            level_pos = level - ( head_node->num_of_levels - skiplist_iter->num_of_levels);
            skiplist_iter = skiplist_iter->lower_lists[level_pos];
        }
    }

    if(skiplist_iter_prev->info & SKIPLIST_LEFT_BORDER_NODE){
        return NULL;
    } else {
        return skiplist_iter_prev->element;;
    }  
}

static
void * skiplist_next(KVSet* kv_set, void * key){
    //TODO NOT THREAD SAFE!
    Skiplist * skiplist = (Skiplist *) &(kv_set->type_specific_data);
    HazardPointerData * hpdata = &skiplist->hazard_pointer_data;
    SkiplistNode * head_node = &(skiplist->head_node);

    struct find_result neigbours = find_neigbours(head_node, 
                                                  key, 
                                                  skiplist->compare, 
                                                  kv_set->key_offset,
                                                  hpdata);
    return neigbours.neigbours_after[SKIPLIST_NUM_OF_LEVELS-1]->element;
}

static
void * skiplist_previous(KVSet* kv_set, void * key){
    //TODO NOT THREAD SAFE!
    Skiplist * skiplist = (Skiplist *) &(kv_set->type_specific_data);
    HazardPointerData * hpdata = &skiplist->hazard_pointer_data;
    SkiplistNode * head_node = &(skiplist->head_node);

    struct find_result neigbours = find_neigbours(head_node, 
                                                  key, 
                                                  skiplist->compare, 
                                                  kv_set->key_offset,
                                                  hpdata);
    return neigbours.neigbours_before[SKIPLIST_NUM_OF_LEVELS-1]->element;
}



/*
 *=================
 * Public interface
 *=================
 */
KVSet * new_skiplist(int (*compare_function)(void *, void *), 
                     void (*free_function)(void *),
                     void *(*malloc_function)(size_t),
                     unsigned int key_offset){
    int i;

    KVSet* kv_set = 
        (KVSet*)malloc_function(sizeof(KVSet) +
                                sizeof(Skiplist) + 
                                sizeof(SkiplistNode*) * (SKIPLIST_NUM_OF_LEVELS));

    KVSetFunctions funs =
        {
            skiplist_delete,
            skiplist_put,
            skiplist_put_new,
            skiplist_remove,
            skiplist_lookup,
            member,
            skiplist_first,
            skiplist_last,    
            skiplist_next,
            skiplist_previous
        };

    Skiplist * skiplist = (Skiplist *) &(kv_set->type_specific_data);

    SkiplistNode* rightmost_skiplist =
        create_skiplist_node(SKIPLIST_NUM_OF_LEVELS, NULL, malloc_function);

    SkiplistNode* leftmost_skiplist = (SkiplistNode*)&(skiplist->head_node);

    kv_set->funs = funs;
    kv_set->key_offset = key_offset;
    
    skiplist->compare = compare_function;
    skiplist->free = free_function;
    skiplist->malloc = malloc_function;
    skiplist->end_node = rightmost_skiplist;

    leftmost_skiplist->element = NULL;
    leftmost_skiplist->num_of_levels = SKIPLIST_NUM_OF_LEVELS;
    
    for(i = 0 ; i < SKIPLIST_NUM_OF_LEVELS ; i++){
        leftmost_skiplist->lower_lists[i] =
            rightmost_skiplist;
    }

    leftmost_skiplist->info = SKIPLIST_LEFT_BORDER_NODE;

    leftmost_skiplist->marked = 0;
    leftmost_skiplist->fully_linked = 1;
    pthread_mutex_init (&leftmost_skiplist->lock, NULL);
    rightmost_skiplist->info = SKIPLIST_RIGHT_BORDER_NODE;

    rightmost_skiplist->marked = 0;
    rightmost_skiplist->fully_linked = 1;

    hazard_pointer_initialize(&skiplist->hazard_pointer_data);
   
    return kv_set;
}

KVSet * new_skiplist_default(){
    return new_skiplist(default_compare_function, 
                        free,
                        malloc,
                        0);
}

static
void random_seed_destructor(void* mem){
    free(mem);
}

void kvset_init(){
    pthread_key_create(&random_seed_key, random_seed_destructor);
}

