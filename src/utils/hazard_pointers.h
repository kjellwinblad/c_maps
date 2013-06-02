#include "pthread.h"

#ifndef HAZARD_POINTERS_H_
#define HAZARD_POINTERS_H_



#define MAX_NUM_OF_POINTERS_PER_THREAD 64
#define RELEASE_LIST_MAX_SIZE 32


typedef struct HPReleaseDataImpl {
   void * data;
   /* 0 means this slot is free to use */
   /* 1 waiting to be freed */
   /* 2 it is not ok to free during this scan */
    int status;
} HPReleaseData;

typedef struct ThreadHazardPointerDataImpl {
    void * hazard_pointers[MAX_NUM_OF_POINTERS_PER_THREAD];
    int current_hp_slot;
    HPReleaseData release_list[RELEASE_LIST_MAX_SIZE];
    int current_rl_slot;
    int release_list_size;
    struct ThreadHazardPointerDataImpl * next;
} ThreadHazardPointerData;


typedef struct HazardPointerDataImpl {
    pthread_key_t my_hazard_pointer_data;
    ThreadHazardPointerData * thread_data_list_first;
} HazardPointerData;


void hazard_pointer_initialize(HazardPointerData * data);

void * hazard_pointer_add(HazardPointerData * data, void ** pointer);

void * hazard_pointer_set(HazardPointerData * data, int slot, void ** pointer);

void * hazard_pointer_set_check(HazardPointerData * data, int slot, void ** pointer, int pos_to_check);

void * hazard_pointer_move_set(HazardPointerData * data, 
                               int move_prev_slot_value_to_slot, 
                               int slot,
                               void ** pointer);

void * hazard_pointer_move_set_check(HazardPointerData * data, 
                               int move_prev_slot_value_to_slot, 
                               int slot,
                               void ** pointer,
                               int pos_to_check);

void hazard_pointer_remove(HazardPointerData * data, void * pointer);

void hazard_pointer_remove_all(HazardPointerData * data, int upto);

void hazard_pointer_free(HazardPointerData * data, void * pointer, void (*free)(void *));

void hazard_pointer_free_everything(HazardPointerData * data, void (*free)(void *));

#endif
