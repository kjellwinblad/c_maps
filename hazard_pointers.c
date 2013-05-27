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
    // continue searching while [imin,imax] is not empty
    while (imax >= imin){
        /* calculate the midpoint for roughly equal partition */
        int imid = imin + (imax-imin)/2;
        // determine which subarray to search
        if (release_list[imid].data < pointer){
            // change min index to search upper subarray
            imin = imid + 1;
        }else if (release_list[imid].data > pointer){
            // change max index to search lower subarray
            imax = imid - 1;
        }else{
            // key found at index imid
            return imid;
        }
    }
    // key not found
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
    //printf("START CHECK LOOP\n");
    while(local_data_iter != NULL){
        for(i = 0; i < MAX_NUM_OF_POINTERS_PER_THREAD; i++){
            current_hp = local_data_iter->hazard_pointers[i];
            //printf("START FIND INDEX\n");
            find_index = find_index_of_pointer(release_list, current_hp);
            //printf("END FIND INDEX\n");
            if(find_index != -1){
                release_list[find_index].status = 2;
            }
        }
        local_data_iter = local_data_iter->next;
    }
    //printf("END CHECK LOOP\n");
    for(i = 0; i < RELEASE_LIST_MAX_SIZE; i++){
        //assert(release_list[i].status != 0);
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
    myData->hp_count = 0;
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

void debug_print_hps(HazardPointerData * data){
    int i, actual;
    actual = 0;
    ThreadHazardPointerData * myData = pthread_getspecific(data->my_hazard_pointer_data);

    if(myData == NULL) {
            printf("NULL!!!\n");
        return;
    }
    printf("HPS===============================\n");
    printf("COUNT %d\n", myData->hp_count);
    for(i = 0;i < MAX_NUM_OF_POINTERS_PER_THREAD;i++){
        printf("pointer %lu\n", myData->hazard_pointers[i]);
        if(myData->hazard_pointers[i]!=NULL)actual++;

    }
    assert(myData->hp_count == actual);
    printf("HPS###############################\n");
}

int get_hazard_count(HazardPointerData * data){
    ThreadHazardPointerData * myData = pthread_getspecific(data->my_hazard_pointer_data);
    if(myData == NULL){
        return 0;
    }else{
        return myData->hp_count;
    }
}

void * hazard_pointer_add(HazardPointerData * data, void ** pointer){
    int slot;
    ThreadHazardPointerData * myData = pthread_getspecific(data->my_hazard_pointer_data);
    if(myData==NULL){
        myData = hazard_pointer_thread_initialize(data);
        pthread_setspecific(data->my_hazard_pointer_data, myData);
        //printf("%myData lu\n", myData);
    }
    //printf("a %d ",myData->hp_count);
    slot = myData->current_hp_slot;
    //printf("before %lu\n", ACCESS_ONCE(*pointer));
    do{
        slot++;
        if(slot == MAX_NUM_OF_POINTERS_PER_THREAD){
            slot = 0;
        }
    }while(myData->hazard_pointers[slot] != NULL);
    //printf("after %lu\n", ACCESS_ONCE(*pointer));
    do{
        myData->hazard_pointers[slot] = ACCESS_ONCE(*pointer);
        __sync_synchronize();
    }while(myData->hazard_pointers[slot] != ACCESS_ONCE(*pointer));
    myData->current_hp_slot = slot;
    myData->hp_count++;
    if(myData->hp_count > 63){
        assert(0);
    }

    //debug_print_hps(data);
    return myData->hazard_pointers[slot];
}

void hazard_pointer_remove(HazardPointerData * data, void * pointer){
    int slot, beginning_slot;
    int c = 0;
    ThreadHazardPointerData * myData = pthread_getspecific(data->my_hazard_pointer_data);
    //printf("r %d ",myData->hp_count);
    beginning_slot = slot = myData->current_hp_slot;
    
    do{ 
        c++;
        if(slot == 0){
            slot = MAX_NUM_OF_POINTERS_PER_THREAD;
        }
        slot = slot - 1;
        //  printf("slot %d %lu  ",slot, myData->hazard_pointers[slot]);
        //
        if(c == 100){
            printf("LOOKING FOR PTR %lu \n",pointer);
            debug_print_hps(data);
            assert(0);
         }
    }while(myData->hazard_pointers[slot] != pointer);
    // printf("hej");
    myData->hazard_pointers[slot] = NULL;
    myData->hp_count--;
    //debug_print_hps(data);
    __sync_synchronize();
}


void hazard_pointer_free(HazardPointerData * data, void * pointer, void (*free)(void *)){
    
    int slot;
    ThreadHazardPointerData * myData = pthread_getspecific(data->my_hazard_pointer_data);
    //printf("MYDATA %lu\n", myData);
    slot = myData->current_rl_slot;
    do{
        slot = slot + 1;
        if(slot == RELEASE_LIST_MAX_SIZE){
            slot = 0;
        }
        //  printf("slot %d\n", slot);
    }while(myData->release_list[slot].status != 0);
    myData->current_rl_slot = slot;
    myData->release_list[slot].data = pointer;
    myData->release_list[slot].status = 1;
    myData->release_list_size = myData->release_list_size + 1;
    if(myData->release_list_size == RELEASE_LIST_MAX_SIZE){
        //printf("DO SCAN!!!!!!!!!!!!!!!!!\n");
        do_free_scan(data, myData,free);
    }
}

void hazard_pointer_free_everything(HazardPointerData * data, void (*free)(void *)){
    ThreadHazardPointerData * local_data_iter = data->thread_data_list_first;
    ThreadHazardPointerData * local_data_iter_prev;  
        while(local_data_iter != NULL){
        do_free_scan(data, local_data_iter,free);
        local_data_iter = local_data_iter->next;
    }
    local_data_iter = data->thread_data_list_first;
    while(local_data_iter != NULL){
        local_data_iter_prev = local_data_iter;
        local_data_iter = local_data_iter->next;
        free(local_data_iter_prev);
    }
}
