#include "pthread.h"

#ifndef HAZARD_POINTERS_H_
#define HAZARD_POINTERS_H_



#define MAX_NUM_OF_POINTERS_PER_THREAD 32
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
    int next_hp_slot;
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

void hazard_pointer_add(HazardPointerData * data, void ** pointer);

void hazard_pointer_remove(HazardPointerData * data, void * pointer);

void hazard_pointer_free(HazardPointerData * data, void * pointer, void (*free)(void *));

#endif /* HAZARD_H_ */
