
#include <stdlib.h>
#include <stdio.h>
#include "ADTVector.h"
#include "ADTList.h"
#include "state.h"


// Οι ολοκληρωμένες πληροφορίες της κατάστασης του παιχνιδιού.
// Ο τύπος State είναι pointer σε αυτό το struct, αλλά το ίδιο το struct
// δεν είναι ορατό στον χρήστη.

struct state {
	Vector objects;			// περιέχει στοιχεία Object (Εμπόδια / Εχθροί / Πύλες)
	List portal_pairs;		// περιέχει PortalPair (ζευγάρια πυλών, είσοδος/έξοδος)

	struct state_info info;
};

// Ζευγάρια πυλών

typedef struct portal_pair {
	Object entrance;		// η πύλη entrance
	Object exit;			// οδηγεί στην exit
}* PortalPair;

ListNode last_as_exit;

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

	character->rect.width = 70;
	character->rect.height = 38;
	character->rect.x = 0;
	character->rect.y = - character->rect.height;

	// Δημιουργία των objects (πύλες / εμπόδια / εχθροί) και προσθήκη στο vector
	// state->objects. Η πίστα περιέχει συνολικά 4*PORTAL_NUM αντικείμενα, από
	// τα οποία τα PORTAL_NUM είναι πύλες, και τα υπόλοια εμπόδια και εχθροί.

	state->objects = vector_create(0, free);		// Δημιουργία του vector

	for (int i = 0; i < 4*PORTAL_NUM; i++) {
		// Δημιουργία του Object και προσθήκη στο vector
		Object obj = malloc(sizeof(*obj));
		vector_insert_last(state->objects, obj);

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
	}

	state->portal_pairs = list_create(free);

	for (int i = 3 ; i < 4*PORTAL_NUM ; i += 4){

		PortalPair pair = malloc(sizeof(*pair));
		int randnum = rand()%(4*PORTAL_NUM);
		pair->entrance = vector_get_at(state->objects, i);
		if(randnum % 4 != 3){
			randnum += 3 - randnum%4;
		}
		
		ListNode node = list_first(state->portal_pairs);
		if (node != LIST_BOF){

			while(node != LIST_EOF){

				if(((PortalPair)list_node_value(state->portal_pairs, node))->exit == ((Object)vector_get_at(state->objects, randnum))){
					randnum = (randnum + 4)% (4*PORTAL_NUM);
					node = list_first(state->portal_pairs);
				}
				else{
					node = list_next(state->portal_pairs, node);
				}

			}

		}
		
		pair->exit = vector_get_at(state->objects, randnum);
		list_insert_next(state->portal_pairs, list_last(state->portal_pairs), pair);
		
		
	}
	
	for (ListNode last_as_exit_node = list_first(state->portal_pairs) ; 
	last_as_exit_node != LIST_EOF ; 
	last_as_exit_node = list_next(state->portal_pairs, last_as_exit_node)){
		if (((PortalPair)list_node_value(state->portal_pairs, last_as_exit_node))->exit == ((PortalPair)list_last(state->portal_pairs))->entrance){
			last_as_exit = last_as_exit_node;
		}
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
	
	for(int i = 0; i < vector_size(state->objects) ; i++){
		Object currobj = vector_get_at(state->objects, i);
		float k = currobj->rect.x;
		
		if (k >= x_from && k <= x_to){
			list_insert_next(objx, list_last(objx), currobj);
		}
		
	}
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
		for(int i = 0 ; i < vector_size(state->objects) ; i++){					// Ελεγχω ολα τα objects
			Object currobj = vector_get_at(state->objects, i);
			if (currobj->type != PORTAL){													// Ελεγχοι για συγκρουσεις χαρακτηρα-εμποδιου/εχθρου
				if (CheckCollisionRecs(currobj->rect, state->info.character->rect)){	// Αν υπαρχει Collision με εχθρο ή εμποδιο
					state->info.playing = false;								// Game Over
				}
			}
			//Ελεγχος για Συγκρουση εχθρου
			if (currobj->type == ENEMY){
				
				if (currobj->forward == true){			//Αν κινειται δεξια
					currobj->rect.x += 5;
				}
				else{
					currobj->rect.x -= 5;					//Αν κινειται αριστερα
				}
				
				for (int j = 0 ; j < vector_size(state->objects) ; j++){		//Ελεγχος για τα υπολοιπα objects
					Object currobj2 = vector_get_at(state->objects, j);
					if (currobj2->type == OBSTACLE){							
						if (CheckCollisionRecs(currobj->rect, currobj2->rect)){		//Αν συγκρουστηκε με εμποδιο αλλαζει κατευθυνση
							currobj->forward = !(currobj->forward);
						}
					}
					else if(currobj2->type == PORTAL){
						
						if (CheckCollisionRecs(currobj2->rect, currobj->rect)){		// Αν συγκρουστηκε με PORTAL
							if(currobj->forward == true){						// Αν κινειται δεξια βρισκουμαι το Portal στη λιστα σαν "entrance" για να οδηγηθει στο καταλληλο exit
								for(ListNode node = list_first(state->portal_pairs) ; node != LIST_EOF ; node = list_next(state->portal_pairs, node)){
									PortalPair currport;
									if ( (currport = list_node_value(state->portal_pairs, node))->entrance != currobj2)
										continue;

									currobj->rect.x = currport->exit->rect.x + (currport->exit->rect.width + 1);
								}
							
							}
							else{												// Αν κινειται αριστερα βρισκουμαι το Portal στη λιστα σαν "exit" για να οδηγηθει στο καταλληλο entrance

								for(ListNode node = list_first(state->portal_pairs) ; node != LIST_EOF ; node = list_next(state->portal_pairs, node)){
									PortalPair currport;
									if ( (currport = list_node_value(state->portal_pairs, node))->exit != currobj2)
										continue;

									currobj->rect.x = currport->entrance->rect.x - (currobj->rect.width + 1);
								}
							
							}
						}
						
					}
				}
			}
		}
		//Ελεγχος για συγκρουση Χαρακτηρα-Portal
		if(state->info.character->forward == true){				//Αν ο χαρακτηρας πηγαινε δεξια
			for(ListNode node = list_first(state->portal_pairs) ; node != LIST_EOF ; node = list_next(state->portal_pairs, node)){
				PortalPair currport;
				if ( CheckCollisionRecs((currport = list_node_value(state->portal_pairs, node))->entrance->rect, state->info.character->rect)){	//Ψαχνουμε το Portal στη λιστα στα entrances για να οδηγηθουμε στο αντιστοιχο exit
					if(node != list_last(state->portal_pairs)){
						state->info.character->rect.x = currport->exit->rect.x + (currport->exit->rect.width + 1);
					}
					else{										//Αν ειναι το τελευταιο Portal νικαει και ξαναρχιζει το παιχνιδι
						state->info.wins++;
						state->info.character->rect.x = 0;
						state->info.character->rect.y = - state->info.character->rect.height;
					}
				}
				
			}
		
		}
		else{													//Αν ο χαρακτηρας πηγαινε αριστερα

			for(ListNode node = list_first(state->portal_pairs) ; node != LIST_EOF ; node = list_next(state->portal_pairs, node)){
				PortalPair currport;
				if ( CheckCollisionRecs((currport = list_node_value(state->portal_pairs, node))->exit->rect, state->info.character->rect)){	//Ψαχνουμε το Portal στη λιστα στα entrances για να οδηγηθουμε στο αντιστοιχο exit
					if(node != last_as_exit){
						state->info.character->rect.x = currport->entrance->rect.x - (state->info.character->rect.width + 1);
					}
					else{										//Αν ειναι το τελευταιο Portal νικαει και ξαναρχιζει το παιχνιδι
						state->info.wins++;
						state->info.character->rect.x = 0;
						state->info.character->rect.y = - state->info.character->rect.height;
					}
				}
			}
		
		}

	}
	//Press Enter Key after game over
	if (state->info.playing == false && keys->enter == true){
		state->info.playing = true;
		state->info.character->rect.x = 0;
		state->info.character->rect.y = - state->info.character->rect.height;
		state->info.character->jumping = false;
		state->info.character->forward = true;
		state->info.wins = 0;
		for (int i = 0 ; i < vector_size(state->objects) ; i++){
			Object value = vector_get_at(state->objects, i);
			value->rect.x = (i + 1) * SPACING;
		}
	}
}

// Καταστρέφει την κατάσταση state ελευθερώνοντας τη δεσμευμένη μνήμη.

void state_destroy(State state) {
	vector_destroy(state->objects);
	list_destroy(state->portal_pairs);
	free(state->info.character);
	free(state);
}