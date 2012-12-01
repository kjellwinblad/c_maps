#include "skiplist.h"
#include "stdlib.h"
#include "stdio.h"

static inline SkipList* create_skiplist_node(int num_of_levels, 
                                             SkiplistElement element){
    SkipList* skiplist = 
        (SkipList*)malloc(sizeof(SkipList) + 
                          sizeof(SkipList*) * (num_of_levels));
    skiplist->element = element;
    skiplist->num_of_levels = num_of_levels;
    return skiplist;
}

static inline int random_level(){
    int i;
    long random_number = rand();
    int num = 2;
    for(i = SKIPLIST_NUM_OF_LEVELS -1 ; i > 0 ; i--){
        if(random_number > (RAND_MAX / num)){
            return i;
        }
        num = num * 2;
    }
    return 0;
}

struct one_level_find_result {
    SkipList * neigbour_before;
    SkipList * element_skiplist;
    SkipList * neigbour_after;    
};

struct find_result {
    SkipList * element_skiplist;
    SkipList * neigbours_before[SKIPLIST_NUM_OF_LEVELS];
    SkipList * neigbours_after[SKIPLIST_NUM_OF_LEVELS];    
};

static inline 
struct one_level_find_result find_neigbours_1_level(SkipList* skiplist,
                                                    SkiplistElement element,
                                                    int level){
#define SMALLER -1
#define EQUAL 0
#define GREATER 1

    int cmp_result;
    SkipList* skiplist_prev = skiplist;
    int level_pos = level - (SKIPLIST_NUM_OF_LEVELS - skiplist_prev->num_of_levels);
    SkipList* skiplist_next = skiplist_prev->lower_lists[level_pos];
    struct one_level_find_result result;

    do{
        cmp_result = SKIPLIST_CMP_ELEMENTS(skiplist_next->element, element);
        if(GREATER == cmp_result){
            result.neigbour_before = skiplist_prev;
            result.element_skiplist = NULL;
            result.neigbour_after = skiplist_next;
        } else if(EQUAL == cmp_result){
            level_pos = level - (SKIPLIST_NUM_OF_LEVELS - skiplist_next->num_of_levels);
            result.neigbour_before = skiplist_prev;
            result.element_skiplist = skiplist_next;
            result.neigbour_after = skiplist_next->lower_lists[level_pos];
        } else {
            level_pos = level - (SKIPLIST_NUM_OF_LEVELS - skiplist_next->num_of_levels);
            skiplist_prev = skiplist_next;
            skiplist_next = skiplist_next->lower_lists[level_pos];
        }
    } while(SMALLER == cmp_result);

    return result;

}

static inline 
struct find_result find_neigbours(SkipList* skiplist,
                                  SkiplistElement element){
    int level;
    struct find_result result;
    struct one_level_find_result level_result;
    SkipList* neigbour_before_iter = skiplist;

    for(level = 0; level < SKIPLIST_NUM_OF_LEVELS; level++){
        level_result = find_neigbours_1_level(neigbour_before_iter, element, level);
        result.neigbours_before[level] = level_result.neigbour_before;
        result.neigbours_after[level] = level_result.neigbour_after;
        neigbour_before_iter = level_result.neigbour_before;
    }
    result.element_skiplist = level_result.element_skiplist;

    return result;

}

static inline void set_next_at_level(SkipList* skiplist,
                                     SkipList* next_skiplist,
                                     int level){
    int level_pos = level - (SKIPLIST_NUM_OF_LEVELS - skiplist->num_of_levels);
    skiplist->lower_lists[level_pos] = next_skiplist;
}

inline void insert_sublist(SkipList* skiplist, 
                           struct find_result neigbours, 
                           SkipList* sublist, 
                           int level){
    int insert_level;
    for(insert_level = level; insert_level < SKIPLIST_NUM_OF_LEVELS; insert_level++){
        set_next_at_level(neigbours.neigbours_before[insert_level],
                          sublist,
                          insert_level);
        set_next_at_level(sublist,
                          neigbours.neigbours_after[insert_level],
                          insert_level);
    }
}

/*
 * Public interface
 */
SkipList* skiplist_new(){
    
    int i;

    SkipList* leftmost_skiplist =
        create_skiplist_node(SKIPLIST_NUM_OF_LEVELS, SKIPLIST_MIN_ELEMENT);

    SkipList* rightmost_skiplist =
        create_skiplist_node(SKIPLIST_NUM_OF_LEVELS, SKIPLIST_MAX_ELEMENT);
    
    for(i = 0 ; i < SKIPLIST_NUM_OF_LEVELS ; i++){
        leftmost_skiplist->lower_lists[i] =
            rightmost_skiplist;
    }
   
    return leftmost_skiplist;

}


void skiplist_delete(SkipList* skiplist){

    SkipList* skiplist_temp = skiplist;
    SkipList* skiplist_iter = skiplist;
    while(skiplist_iter->element != SKIPLIST_MAX_ELEMENT){
        skiplist_temp = skiplist_iter;
        skiplist_iter = skiplist_iter->lower_lists[skiplist_iter->num_of_levels -1];
        free(skiplist_temp);
    }

    free(skiplist_iter);

}


SkiplistElement skiplist_put(SkipList* skiplist, SkiplistElement element){
    SkiplistElement returnValue;
    struct find_result neigbours = find_neigbours(skiplist, element);
    int level = random_level();
    int num_of_elements_in_insert_level = SKIPLIST_NUM_OF_LEVELS - level;
    SkipList* new_skiplist_node =
        create_skiplist_node(num_of_elements_in_insert_level, element);
    insert_sublist(skiplist, neigbours, new_skiplist_node, level);
    if(neigbours.element_skiplist == NULL){
        return SKIPLIST_MIN_ELEMENT;
    } else {
        returnValue = neigbours.element_skiplist->element;
        free(neigbours.element_skiplist);
        return returnValue;
    }
}

int skiplist_put_new(SkipList* skiplist, SkiplistElement element){
    struct find_result neigbours = find_neigbours(skiplist, element);
    int level = random_level();
    int num_of_elements_in_insert_level = SKIPLIST_NUM_OF_LEVELS - level;
    SkipList* new_skiplist_node;
    if(neigbours.element_skiplist == NULL){
        new_skiplist_node =
            create_skiplist_node(num_of_elements_in_insert_level, element);
        insert_sublist(skiplist, neigbours, new_skiplist_node, level);
        return 1;
    } else {
        return 0;
    }
}

SkiplistElement skiplist_remove(SkipList* skiplist, SkiplistElement element){
    struct find_result neigbours = find_neigbours(skiplist, element);
    SkiplistElement returnValue;
    int remove_level;
    int remove_from_level;
    if(neigbours.element_skiplist == NULL){
        return SKIPLIST_MIN_ELEMENT;
    } else {
        remove_from_level = 
            SKIPLIST_NUM_OF_LEVELS - neigbours.element_skiplist->num_of_levels;
        for(remove_level = remove_from_level; remove_level < SKIPLIST_NUM_OF_LEVELS; remove_level++){
            set_next_at_level(neigbours.neigbours_before[remove_level],
                              neigbours.neigbours_after[remove_level],
                              remove_level);
        }
        returnValue = neigbours.element_skiplist->element;
        free(neigbours.element_skiplist);
        return returnValue;
    }
}

SkiplistElement skiplist_lookup(SkipList* skiplist, SkiplistElement element){
    struct find_result neigbours = find_neigbours(skiplist, element);
    SkiplistElement returnValue;
    if(neigbours.element_skiplist == NULL){
        return SKIPLIST_MIN_ELEMENT;
    } else {
        returnValue = neigbours.element_skiplist->element;
        return returnValue;
    }
}


SkiplistElement skiplist_first(SkipList* skiplist){
    SkiplistElement firstCandidate = 
        skiplist->lower_lists[SKIPLIST_NUM_OF_LEVELS - 1]->element;
    if(firstCandidate == SKIPLIST_MAX_ELEMENT){
        return SKIPLIST_MIN_ELEMENT;
    } else {
        return firstCandidate;
    }
}

SkiplistElement skiplist_last(SkipList* skiplist){
    int level;
    int level_pos;
    SkipList* skiplist_iter_prev = skiplist;
    SkipList* skiplist_iter;
    
    for(level = 0; level < SKIPLIST_NUM_OF_LEVELS; level++){
        level_pos = level - (SKIPLIST_NUM_OF_LEVELS - skiplist_iter_prev->num_of_levels);
        skiplist_iter = skiplist_iter_prev->lower_lists[level_pos];
        while(skiplist_iter->element != SKIPLIST_MAX_ELEMENT) {
            skiplist_iter_prev = skiplist_iter;
            level_pos = level - (SKIPLIST_NUM_OF_LEVELS - skiplist_iter->num_of_levels);
            skiplist_iter = skiplist_iter->lower_lists[level_pos];
        }
    }

    SkiplistElement lastCandidate = skiplist_iter_prev->element;
    if(lastCandidate == SKIPLIST_MIN_ELEMENT){
        return SKIPLIST_MAX_ELEMENT;
    } else {
        return lastCandidate;
    }  
}

SkiplistElement skiplist_next(SkipList* skiplist, SkiplistElement element){
    struct find_result neigbours = find_neigbours(skiplist, element);
    return neigbours.neigbours_after[SKIPLIST_NUM_OF_LEVELS-1]->element;
}


SkiplistElement skiplist_previous(SkipList* skiplist, SkiplistElement element){
    struct find_result neigbours = find_neigbours(skiplist, element);
    return neigbours.neigbours_before[SKIPLIST_NUM_OF_LEVELS-1]->element;
}
