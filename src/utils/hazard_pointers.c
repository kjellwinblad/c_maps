#include "hazard_pointers.h"
#include "smp_utils.h"
#include "stdlib.h"
#include "assert.h"


int hp_release_data_compare (const void * a, const void * b){
    const HPReleaseData * da = a;
    const HPReleaseData * db = b;
    if(da->data < db->data){
        return -1;
    }else if(da->data == db->data){
        return 0;
    }else{
        return 1;
    }
}

//Based on http://en.wikipedia.org/wiki/Binary_search_algorithm
//Page last modifed: 26 May 2013 at 06:16
int find_index_of_pointer(HPReleaseData release_list[], void * pointer){
    int imin = 0;
    int imax = RELEASE_LIST_MAX_SIZE - 1;
    while (imax >= imin){
        int imid = imin + (imax-imin)/2;
        if (release_list[imid].data < pointer){
            imin = imid + 1;
        }else if (release_list[imid].data > pointer){
            imax = imid - 1;
        }else{
            return imid;
        }
    }
    return -1;
}

void do_free_scan(HazardPointerData * data, ThreadHazardPointerData * myData, void (*free)(void *)){
    int i;
    void * current_hp;
    int find_index;
    int free_count = 0;
    ThreadHazardPointerData * local_data_iter = data->thread_data_list_first;
    HPReleaseData * release_list = myData->release_list;
    qsort(release_list, RELEASE_LIST_MAX_SIZE, sizeof(HPReleaseData), hp_release_data_compare);
    while(local_data_iter != NULL){
        for(i = 0; i < MAX_NUM_OF_POINTERS_PER_THREAD; i++){
            current_hp = local_data_iter->hazard_pointers[i];
            find_index = find_index_of_pointer(release_list, current_hp);
            if(find_index != -1){
                release_list[find_index].status = 2;
            }
        }
        local_data_iter = local_data_iter->next;
    }
    for(i = 0; i < RELEASE_LIST_MAX_SIZE; i++){
        if(release_list[i].status == 1){
            free(release_list[i].data);
            release_list[i].data = NULL;
            release_list[i].status = 0;
            free_count = free_count + 1;
        }else if(release_list[i].status == 2){
            release_list[i].status = 1;
        }
    }
    myData->release_list_size = myData->release_list_size - free_count;
}

ThreadHazardPointerData * hazard_pointer_thread_initialize(HazardPointerData * data){
    ThreadHazardPointerData * myData = malloc(sizeof(ThreadHazardPointerData));
    int success = 0;
    int i;
    for(i = 0; i < MAX_NUM_OF_POINTERS_PER_THREAD; i++){
        myData->hazard_pointers[i] = NULL;
    }
    myData->current_hp_slot = 0;
    for(i = 0; i < RELEASE_LIST_MAX_SIZE; i++){
        myData->release_list[i].data = NULL;
        myData->release_list[i].status = 0;
    }
    myData->current_rl_slot = RELEASE_LIST_MAX_SIZE - 1;
    myData->release_list_size = 0;
    while(!success){
        myData->next = ACCESS_ONCE(data->thread_data_list_first);
        success = __sync_bool_compare_and_swap(&data->thread_data_list_first, myData->next, myData);
    }
    return myData;
}

void hazard_pointer_initialize(HazardPointerData * data){
    pthread_key_create(&data->my_hazard_pointer_data, NULL/*TODO*/);
    data->thread_data_list_first = NULL;
}

void * hazard_pointer_add(HazardPointerData * data, void ** pointer){
    int slot;
    ThreadHazardPointerData * myData = pthread_getspecific(data->my_hazard_pointer_data);
    if(myData==NULL){
        myData = hazard_pointer_thread_initialize(data);
        pthread_setspecific(data->my_hazard_pointer_data, myData);
    }
    slot = myData->current_hp_slot;
    while(myData->hazard_pointers[slot] != NULL){
        slot++;
        if(slot == MAX_NUM_OF_POINTERS_PER_THREAD){
            slot = 0;
        }
    }
    do{
        myData->hazard_pointers[slot] = ACCESS_ONCE(*pointer);
        __sync_synchronize();
    }while(myData->hazard_pointers[slot] != ACCESS_ONCE(*pointer));
    myData->current_hp_slot = slot;
    return myData->hazard_pointers[slot];
}

void hazard_pointer_remove(HazardPointerData * data, void * pointer){
    int slot;
    ThreadHazardPointerData * myData = pthread_getspecific(data->my_hazard_pointer_data);
    slot = myData->current_hp_slot;
    do{
        if(slot == 0){
            slot = MAX_NUM_OF_POINTERS_PER_THREAD;
        }
        slot = slot - 1;
    }while(myData->hazard_pointers[slot] != pointer);
    myData->hazard_pointers[slot] = NULL;
    myData->current_hp_slot = slot;
    __sync_synchronize();
}


void hazard_pointer_free(HazardPointerData * data, void * pointer, void (*free)(void *)){
    
    int slot;
    ThreadHazardPointerData * myData = pthread_getspecific(data->my_hazard_pointer_data);
    slot = myData->current_rl_slot;
    do{
        slot = slot + 1;
        if(slot == RELEASE_LIST_MAX_SIZE){
            slot = 0;
        }
    }while(myData->release_list[slot].status != 0);
    myData->current_rl_slot = slot;
    myData->release_list[slot].data = pointer;
    myData->release_list[slot].status = 1;
    myData->release_list_size = myData->release_list_size + 1;
    if(myData->release_list_size == RELEASE_LIST_MAX_SIZE){
        do_free_scan(data, myData,free);
    }
}

void hazard_pointer_free_everything(HazardPointerData * data, void (*freeFunc)(void *)){
    ThreadHazardPointerData * local_data_iter = data->thread_data_list_first;
    ThreadHazardPointerData * local_data_iter_prev;  
    while(local_data_iter != NULL){
        do_free_scan(data, local_data_iter,freeFunc);
        local_data_iter = local_data_iter->next;
    }
    local_data_iter = data->thread_data_list_first;
    while(local_data_iter != NULL){
        local_data_iter_prev = local_data_iter;
        local_data_iter = local_data_iter->next;
        free(local_data_iter_prev);
    }
}
