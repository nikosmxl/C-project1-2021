//////////////////////////////////////////////////////////////////
//
// Test για το state.h module
//
//////////////////////////////////////////////////////////////////

#include "acutest.h"			// Απλή βιβλιοθήκη για unit testing

#include "state.h"
#include "ADTVector.h"
#include "ADTList.h"

void test_state_create() {

	State state = state_create();
	TEST_ASSERT(state != NULL);

	StateInfo info = state_info(state);
	TEST_ASSERT(info != NULL);

	TEST_ASSERT(info->current_portal == 0);
	TEST_ASSERT(info->wins == 0);
	TEST_ASSERT(info->playing);
	TEST_ASSERT(!info->paused);

	List objcheck = state_objects(state, 0, 4*PORTAL_NUM*SPACING + 1);
	TEST_ASSERT(list_size(objcheck) == 400);
	int i = 0;
	Pointer prev;
	for (ListNode node = list_first(objcheck) ; node != LIST_EOF ; node = list_next(objcheck, node)){
		if (i != 0){
			TEST_ASSERT(((Object)list_node_value(objcheck, node))->rect.x - ((Object)prev)->rect.x == 700);
		}
		
		prev = list_node_value(objcheck, node);
		i++;
	}

	list_destroy(objcheck);

	objcheck = state_objects(state, 200, 1500);
	for (ListNode node = list_first(objcheck) ; node != LIST_EOF ; node = list_next(objcheck, node)){
		float i = ((Object)list_node_value(objcheck, node))->rect.x;
		TEST_ASSERT(i >= 200 && i <= 1500);
	}

	list_destroy(objcheck);

	objcheck = state_objects(state, 5000, 30000);
	for (ListNode node = list_first(objcheck) ; node != LIST_EOF ; node = list_next(objcheck, node)){
		float i = ((Object)list_node_value(objcheck, node))->rect.x;
		TEST_ASSERT(i >= 5000 && i <= 30000);
	}

	list_destroy(objcheck);

	objcheck = state_objects(state, 700, 3500);
	for (ListNode node = list_first(objcheck) ; node != LIST_EOF ; node = list_next(objcheck, node)){
		float i = ((Object)list_node_value(objcheck, node))->rect.x;
		TEST_ASSERT(i >= 700 && i <= 3500);
	}

	list_destroy(objcheck);

	state_destroy(state);
	
}

void test_state_update() {
	State state = state_create();
	TEST_ASSERT(state != NULL && state_info(state) != NULL);

	// Πληροφορίες για τα πλήκτρα (αρχικά κανένα δεν είναι πατημένο)
	struct key_state keys = { false, false, false, false, false, false };
	
	// Χωρίς κανένα πλήκτρο, ο χαρακτήρας μετακινείται 7 pixels μπροστά
	Rectangle old_rect = state_info(state)->character->rect;
	state_update(state, &keys);
	Rectangle new_rect = state_info(state)->character->rect;
	
	TEST_ASSERT( new_rect.x == old_rect.x + 7 && new_rect.y == old_rect.y );

	// Με πατημένο το δεξί βέλος, ο χαρακτήρας μετακινείται 12 pixes μπροστά
	keys.right = true;
	old_rect = state_info(state)->character->rect;
	state_update(state, &keys);
	new_rect = state_info(state)->character->rect;

	TEST_ASSERT( new_rect.x == old_rect.x + 12 && new_rect.y == old_rect.y );

	old_rect = state_info(state)->character->rect;
	keys.right = false;
	keys.left = true;
	state_update(state, &keys);
	new_rect = state_info(state)->character->rect;

	TEST_ASSERT( new_rect.x == old_rect.x - 12 && new_rect.y == old_rect.y );

	old_rect = state_info(state)->character->rect;
	keys.left = false;
	state_update(state, &keys);
	new_rect = state_info(state)->character->rect;

	TEST_ASSERT( new_rect.x == old_rect.x - 7 && new_rect.y == old_rect.y );
	old_rect = state_info(state)->character->rect;
	keys.up = true;
	state_update(state, &keys);
	new_rect = state_info(state)->character->rect;

	TEST_ASSERT( new_rect.x == old_rect.x - 7 && new_rect.y == old_rect.y - 15 );

	for (int i = 1 ; i <= 12 ; i++)
		state_update(state, &keys);

	new_rect = state_info(state)->character->rect;
	TEST_ASSERT( new_rect.x == old_rect.x - 91 && new_rect.y == old_rect.y - 195 );

	state_update(state, &keys);
	new_rect = state_info(state)->character->rect;

	TEST_ASSERT( new_rect.x == old_rect.x - 98 && new_rect.y == old_rect.y - 180 );

	for (int i = 1 ; i <= 12 ; i++)
		state_update(state, &keys);

	new_rect = state_info(state)->character->rect;
	TEST_ASSERT( new_rect.x == old_rect.x - 182 && new_rect.y == old_rect.y );

	state_update(state, &keys);
	new_rect = state_info(state)->character->rect;

	TEST_ASSERT( new_rect.x == old_rect.x - 189 && new_rect.y == old_rect.y - 15 );

	old_rect = state_info(state)->character->rect;
	keys.p = true;
	state_update(state, &keys);
	new_rect = state_info(state)->character->rect;
	
	TEST_ASSERT( new_rect.x == old_rect.x && new_rect.y == old_rect.y );

	state_info(state)->playing = false;
	keys.enter = true;
	state_update(state, &keys);

	state_destroy(state);

	// Προσθέστε επιπλέον ελέγχους
}


// Λίστα με όλα τα tests προς εκτέλεση
TEST_LIST = {
	{ "test_state_create", test_state_create },
	{ "test_state_update", test_state_update },

	{ NULL, NULL } // τερματίζουμε τη λίστα με NULL
};