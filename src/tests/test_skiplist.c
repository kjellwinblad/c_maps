#include "skiplist.h"
#include "test_kvset.h"

#include "stdio.h"

int main(int argc, char **argv){
    
    printf("\n\n\n\033[32m ### STARTING SKIPLIST TESTS! -- \033[m\n\n\n");

    test_general_kvset_properties(new_skiplist_default);

    test_ordered_kvset_properties(new_skiplist_default);

    printf("\n\n\n\033[32m ### SKIPLIST TESTS COMPLETED! -- \033[m\n\n\n");

    exit(0);

}

