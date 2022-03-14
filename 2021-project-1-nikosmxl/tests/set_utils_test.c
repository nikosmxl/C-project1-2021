#include "acutest.h"
#include <stdio.h>
#include "set_utils.h"

int* create_int(int value) {
	int* pointer = malloc(sizeof(int));		// δέσμευση μνήμης
	*pointer = value;						// αντιγραφή του value στον νέο ακέραιο
	return pointer;
}

int compare_ints(Pointer a, Pointer b){

    return *(int*)a - *(int*)b;
}

int compare_floats(Pointer a, Pointer b){

    return *(float*)a - *(float*)b;
}

int compare_longs(Pointer a, Pointer b){

    return *(long*)a - *(long*)b;
}

int compare_strings(Pointer a, Pointer b){

    return strcmp((char*)a, (char*)b);
}

void test_set_utils(){
    Set set = set_create(compare_ints, free);
    set_insert(set, create_int(3));
    set_insert(set, create_int(7));
    set_insert(set, create_int(7));
    set_insert(set, create_int(0));
    set_insert(set, create_int(13));
    set_insert(set, create_int(145));
    set_insert(set, create_int(1));
    int* curr1 = create_int(13);
    int* y = set_find_eq_or_greater(set, curr1);
    free(curr1);
    TEST_ASSERT(*y == 13);
    int* curr2 = create_int(13434);
    y = set_find_eq_or_greater(set, curr2);
    // if(curr2 != NULL){
    //     printf("hahaha\n");
    //     free(curr2);
    // }
    TEST_ASSERT(y == NULL);
    
    set_destroy(set);

}

TEST_LIST = {
	{ "test_set_utils", test_set_utils },

	{ NULL, NULL } // τερματίζουμε τη λίστα με NULL
};