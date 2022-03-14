
#include <stdlib.h>
#include <stdio.h>

#include "ADTSet.h"

#include "ADTMap.h"

#include "ADTVector.h"

#include "ADTList.h"

#include "state.h"

#include "set_utils.h"

// Οι ολοκληρωμένες πληροφορίες της κατάστασης του παιχνιδιού.
// Ο τύπος State είναι pointer σε αυτό το struct, αλλά το ίδιο το struct
// δεν είναι ορατό στον χρήστη.

struct state {
	Set objects;			// περιέχει στοιχεία Object (Εμπόδια / Εχθροί / Πύλες)
	Map portal_pairs_en;		// περιέχει PortalPair (ζευγάρια πυλών, είσοδος/έξοδος)
	Map portal_pairs_ex;		// περιέχει PortalPair (ζευγάρια πυλών, είσοδος/έξοδος)

	struct state_info info;
};

// Ζευγάρια πυλών

typedef struct portal_pair {
	Object entrance;		// η πύλη entrance
	Object exit;			// οδηγεί στην exit
}* PortalPair;

int compare_x(Pointer a, Pointer b){
	float x = ((Object)a)->rect.x - ((Object)b)->rect.x;
	if (x > 0){
		return 1;
	}
	else if (x == 0){
		return 0;
	}
	else{
		return -1;
	}
}

int compare_y(Pointer a, Pointer b){
	float y = ((Object)a)->rect.y - ((Object)b)->rect.y;
	if (y > 0){
		return 1;
	}
	else if (y == 0){
		return 0;
	}
	else{
		return -1;
	}
}

int compare_set(Pointer a, Pointer b){
	int diff_x = compare_x(a, b);
	if (diff_x != 0){
		return diff_x;					//Ταξινομει πρωτα ως προς x
	}
	else{
		int diff_y = compare_y(a, b);
		if (diff_y != 0){
			return diff_y;				//Αν τα x ειναι ισα ταξινομει ως προς y
		}
		else{
			return (a - b);				// Αν εχουν ιδιες συντεταγμενες τοτε τα διαφοροποιει συγκρινοντας τους pointers
		}
	}
}

Pointer set_get_at(Set set, int i){
	for(SetNode currn = set_first(set) ; currn != SET_EOF ; currn = set_next(set, currn)){
		
		if(i == 0){
			return set_node_value(set, currn);
		}
		i--;
	}
	return NULL;
}

// Δημιουργεί και επιστρέφει την αρχική κατάσταση του παιχνιδιού

State state_create() {
	// Δημιουργία του state
	State state = malloc(sizeof(*state));

	// Γενικές πληροφορίες
	state->info.current_portal = 0;			// Δεν έχουμε περάσει καμία πύλη
	state->info.wins = 0;					// Δεν έχουμε νίκες ακόμα
	state->info.playing = true;				// Το παιχνίδι ξεκινάει αμέσως
	state->info.paused = false;				// Χωρίς να είναι paused.

	// Πληροφορίες για το χαρακτήρα.
	Object character = state->info.character = malloc(sizeof(*character));
	character->type = CHARACTER;
	character->forward = true;
	character->jumping = false;

    // Ο χαρακτήρας (όπως και όλα τα αντικείμενα) έχουν συντεταγμένες x,y σε ένα
    // καρτεσιανό επίπεδο.
	// - Στο άξονα x το 0 είναι η αρχή στης πίστας και οι συντεταγμένες
	//   μεγαλώνουν προς τα δεξιά.
	// - Στον άξονα y το 0 είναι το "δάπεδο" της πίστας, και οι
	//   συντεταγμένες μεγαλώνουν προς τα _κάτω_.
	// Πέρα από τις συντεταγμένες, αποθηκεύουμε και τις διαστάσεις width,height
	// κάθε αντικειμένου. Τα x,y,width,height ορίζουν ένα παραλληλόγραμμο, οπότε
	// μπορούν να αποθηκευτούν όλα μαζί στο obj->rect τύπου Rectangle (ορίζεται
	// στο include/raylib.h).
	// 
	// Προσοχή: τα x,y αναφέρονται στην πάνω-αριστερά γωνία του Rectangle, και
	// τα y μεγαλώνουν προς τα κάτω, οπότε πχ ο χαρακτήρας που έχει height=38,
	// αν θέλουμε να "κάθεται" πάνω στο δάπεδο, θα πρέπει να έχει y=-38.

	character->rect.width = 50;
	character->rect.height = 50;
	character->rect.x = 0;
	character->rect.y = - character->rect.height;

	// Δημιουργία των objects (πύλες / εμπόδια / εχθροί) και προσθήκη στο vector
	// state->objects. Η πίστα περιέχει συνολικά 4*PORTAL_NUM αντικείμενα, από
	// τα οποία τα PORTAL_NUM είναι πύλες, και τα υπόλοια εμπόδια και εχθροί.

	state->objects = set_create(compare_set, NULL);		// Δημιουργία του vector

	for (int i = 0; i < 4*PORTAL_NUM; i++) {
		// Δημιουργία του Object και προσθήκη στο vector
		Object obj = malloc(sizeof(*obj));
		
		// Κάθε 4 αντικείμενα υπάρχει μια πύλη. Τα υπόλοιπα αντικείμενα
		// επιλέγονται τυχαία.

		if(i % 4 == 3) {							// Το 4ο, 8ο, 12ο κλπ αντικείμενο
			obj->type = PORTAL;						// είναι πύλη.
			obj->rect.width = 100;
			obj->rect.height = 5;

		} else if(rand() % 2 == 0) {				// Για τα υπόλοιπα, με πιθανότητα 50%
			obj->type = OBSTACLE;					// επιλέγουμε εμπόδιο.
			obj->rect.width = 10;
			obj->rect.height = 80;

		} else {
			obj->type = ENEMY;						// Και τα υπόλοιπα είναι εχθροί.
			obj->rect.width = 30;
			obj->rect.height = 30;
			obj->forward = false;					// Οι εχθροί αρχικά κινούνται προς τα αριστερά.
		}

		// Τα αντικείμενα είναι ομοιόμορφα τοποθετημένα σε απόσταση SPACING
		// μεταξύ τους, και "κάθονται" πάνω στο δάπεδο.

		obj->rect.x = (i+1) * SPACING;
		obj->rect.y = - obj->rect.height;

		set_insert(state->objects, obj);
	}

	state->portal_pairs_en = map_create(compare_set, NULL, NULL);
	state->portal_pairs_ex = map_create(compare_set, NULL, NULL);
	//Ξεκιναμε απο τον τριτο κομβο του
	SetNode nodeset = set_next(state->objects, set_next(state->objects, set_next(state->objects, set_first(state->objects))));	// Αρχιζω απο το 4ο Object
	for (int i = 3 ; i < 4*PORTAL_NUM ; i += 4){

		PortalPair pair = malloc(sizeof(*pair));
		int randnum = rand()%(4*PORTAL_NUM);

		pair->entrance = set_node_value(state->objects, nodeset);
		if(randnum % 4 != 3){
			randnum += 3 - randnum%4;
		}
		
		MapNode node = map_first(state->portal_pairs_en);

		while(node != MAP_EOF){

			if(((Object)map_node_value(state->portal_pairs_en, node)) == ((Object)set_get_at(state->objects, randnum))){
				randnum = (randnum + 4) % (4*PORTAL_NUM);
				node = map_first(state->portal_pairs_en);
			}
			else{
				node = map_next(state->portal_pairs_en, node);
			}

		}
		
		pair->exit = set_get_at(state->objects, randnum);
		map_insert(state->portal_pairs_en, pair->entrance, pair->exit);
		map_insert(state->portal_pairs_ex, pair->exit, pair->entrance);

			for (int j = 1 ; j <= 4 ; j++)						// Προχοραω καθε φορα 4 objects
				if (nodeset != set_last(state->objects))
					nodeset = set_next(state->objects, nodeset);
		
		free(pair);
	}

	return state;
}

// Επιστρέφει τις βασικές πληροφορίες του παιχνιδιού στην κατάσταση state

StateInfo state_info(State state) {
	
	return &(state->info);
}

// Επιστρέφει μια λίστα με όλα τα αντικείμενα του παιχνιδιού στην κατάσταση state,
// των οποίων η συντεταγμένη x είναι ανάμεσα στο x_from και x_to.

List state_objects(State state, float x_from, float x_to) {
	List objx = list_create(NULL);
	
	Object curr1 = malloc(sizeof(*curr1));
	curr1->rect.x = x_from;
	curr1->rect.y = __FLT_MIN__ ;

	Object curr2 = malloc(sizeof(*curr2));
	curr2->rect.x = x_to;
	curr2->rect.y = __FLT_MAX__ ;
	
	Object loop_start = set_find_eq_or_greater(state->objects, curr1);		// Object x_from
	Object loop_end = set_find_eq_or_smaller(state->objects, curr2);		// Object x_to

	if (loop_start == NULL || loop_end == NULL){
		free(curr1);
		free(curr2);
		return objx;
	}
	
	SetNode loop_start_node = set_find_node(state->objects, loop_start);
	SetNode loop_end_node = set_next(state->objects, set_find_node(state->objects, loop_end));

	for(SetNode i = loop_start_node ; i != loop_end_node ; i = set_next(state->objects, i)){
		list_insert_next(objx, list_last(objx), set_node_value(state->objects, i));
	}

	free(curr1);
	free(curr2);
	
	return objx;
}

// Ενημερώνει την κατάσταση state του παιχνιδιού μετά την πάροδο 1 frame.
// Το keys περιέχει τα πλήκτρα τα οποία ήταν πατημένα κατά το frame αυτό.

void state_update(State state, KeyState keys) {

	if (state->info.playing == true){
		//Pause Key
		if (keys->p == true){
			state->info.paused = !state->info.paused;
		}
		//Press n Key while paused
		if (state->info.paused == true){
			if (keys->n == false){
				return;
			}
		}

		//Left or Right Keys
		if (keys->right){
			state->info.character->forward = true;
		}
		else if (keys->left){
			state->info.character->forward = false;
		}
		//Movement
		if (state->info.character->forward == true){
			if (keys->right){
				state->info.character->rect.x += 12;
			}
			else{
				state->info.character->rect.x += 7;
			}
		}
		else{
			if (keys->left){
				state->info.character->rect.x -= 12;
			}
			else{
				state->info.character->rect.x -= 7;
			}
		}

		//Jumping (Up Key)
		if (state->info.character->jumping == true){

			if (state->info.character->rect.y > -220){
				state->info.character->rect.y -= 15;
			}
			else{
				state->info.character->jumping = false;
				state->info.character->rect.y += 15;
			}

		}
		else if (state->info.character->rect.y < - state->info.character->rect.height){
			state->info.character->rect.y += 15;
		}
		else if (keys->up == true){
			state->info.character->jumping = true;
			state->info.character->rect.y -= 15;
		}
		
		//Ελεγχος για τα υπολοιπα αντικειμενα και τις συγκρουσεις.
		
		List around_char = state_objects(state, state->info.character->rect.x - 100, state->info.character->rect.x + state->info.character->rect.width);

		for(ListNode node = list_first(around_char) ; node != LIST_EOF ; node = list_next(around_char, node)){
			Object obj_to_check = list_node_value(around_char, node);
			if (obj_to_check->type != PORTAL){
				if (CheckCollisionRecs(obj_to_check->rect, state->info.character->rect)){	// Αν υπαρχει Collision με εχθρο ή εμποδιο
					state->info.playing = false;											// Game Over
				}
			}
			else{									//Ελεγχος για συγκρουση Χαρακτηρα-Portal
				
				if(state->info.character->forward == true){
					if (CheckCollisionRecs(obj_to_check->rect, state->info.character->rect)){
						MapNode nodemap = map_find_node(state->portal_pairs_en, obj_to_check);
						if (map_next(state->portal_pairs_en, nodemap) == MAP_EOF){
							state->info.wins++;
							state->info.character->rect.x = 0;
							state->info.character->rect.y = - state->info.character->rect.height;
						}
						else{
							state->info.character->rect.x = ((Object)map_node_value(state->portal_pairs_en, nodemap))->rect.x + (obj_to_check->rect.width + 1);
						}
					}
				}
				else{
					if (CheckCollisionRecs(obj_to_check->rect, state->info.character->rect)){
						MapNode nodemap = map_find_node(state->portal_pairs_ex, obj_to_check);
						if (map_next(state->portal_pairs_ex, nodemap) == MAP_EOF){
							state->info.wins++;
							state->info.character->rect.x = 0;
							state->info.character->rect.y = - state->info.character->rect.height;
						}
						else{
							state->info.character->rect.x = ((Object)map_node_value(state->portal_pairs_ex, nodemap))->rect.x - (state->info.character->rect.width + 1);
						}
					}
				}

			}
		}
		list_destroy(around_char);
		
		List to_update = list_create(NULL);
		
		for(SetNode obj = set_first(state->objects) ; obj != SET_EOF ; obj = set_next(state->objects, obj)){
			list_insert_next(to_update, list_last(to_update), set_node_value(state->objects, obj));
		}

		for(ListNode obj = list_first(to_update) ; obj != LIST_EOF ; obj = list_next(to_update, obj)){					// Ελεγχω ολα τα objects
			Object to_move_obj = list_node_value(to_update, obj);
			
			//Ελεγχος για Συγκρουση εχθρου
			if (to_move_obj->type == ENEMY){
				set_remove(state->objects, to_move_obj);
				if (to_move_obj->forward == true){			//Αν κινειται δεξια
					to_move_obj->rect.x += 5;
				}
				else{										//Αν κινειται αριστερα
					to_move_obj->rect.x -= 5;
				}
				set_insert(state->objects, to_move_obj);
				List around_enemy = state_objects(state, to_move_obj->rect.x - 100, to_move_obj->rect.x + to_move_obj->rect.width);
				
				for(ListNode node = list_first(around_enemy) ; node != LIST_EOF ; node = list_next(around_enemy, node)){

					Object obj_to_check = list_node_value(around_enemy, node);
					if (obj_to_check->type == OBSTACLE){
						if (CheckCollisionRecs(obj_to_check->rect, to_move_obj->rect)){				
							to_move_obj->forward = !(to_move_obj->forward);							
						}
					}
					else if (obj_to_check->type == PORTAL){
						if (CheckCollisionRecs(obj_to_check->rect, to_move_obj->rect)){
							set_remove(state->objects, to_move_obj);
							if (to_move_obj->forward == true){
								MapNode nodemap = map_find_node(state->portal_pairs_en, obj_to_check);
								to_move_obj->rect.x = ((Object)map_node_value(state->portal_pairs_en, nodemap))->rect.x + (obj_to_check->rect.width + 1);
							}
							else{
								MapNode nodemap = map_find_node(state->portal_pairs_ex, obj_to_check);
								to_move_obj->rect.x = ((Object)map_node_value(state->portal_pairs_ex, nodemap))->rect.x - (to_move_obj->rect.width + 1);
							}
							set_insert(state->objects, to_move_obj);
						}
					}

				}
				list_destroy(around_enemy);

			}
			
		}
		
		list_destroy(to_update);
		
	}
	//Press Enter Key after game over
	if (state->info.playing == false && keys->enter == true){
		state->info.playing = true;
		state->info.character->rect.x = 0;
		state->info.character->rect.y = - state->info.character->rect.height;
		state->info.character->jumping = false;
		state->info.character->forward = true;
		state->info.wins = 0;

		List previous_x = list_create(NULL);

		for (SetNode i = set_first(state->objects) ; i != SET_EOF ; i = set_next(state->objects, i)){
			list_insert_next(previous_x, list_last(previous_x), set_node_value(state->objects, i));
		}
		
		int j = 0;
		for (ListNode i = list_first(previous_x) ; i != LIST_EOF ; i = list_next(previous_x, i)){
			
			Object value = list_node_value(previous_x, i);
			
			set_remove(state->objects, value);
			value->rect.x = (j+1) * SPACING;
			set_insert(state->objects, value);
			j++;
		}

		list_destroy(previous_x);
	}
	
}

// Καταστρέφει την κατάσταση state ελευθερώνοντας τη δεσμευμένη μνήμη.

void state_destroy(State state) {
	set_set_destroy_value(state->objects, free);
	set_destroy(state->objects);
	map_destroy(state->portal_pairs_en);
	map_destroy(state->portal_pairs_ex);
	free(state->info.character);
	free(state);
}