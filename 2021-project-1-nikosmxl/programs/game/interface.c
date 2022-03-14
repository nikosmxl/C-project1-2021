#include "raylib.h"

#include "state.h"

#include "interface.h"

#include <stdio.h>

#define SCREEN_WIDTH 850
#define SCREEN_HEIGHT 500

#define CHAR_POS_X (SCREEN_WIDTH/2- info->character->rect.width/2)
#define GROUND_LINE (SCREEN_HEIGHT -70)

#define X_FROM (info->character->rect.x - CHAR_POS_X)
#define X_TO (info->character->rect.x - CHAR_POS_X + SCREEN_WIDTH)

// Assets
Texture court;
Sound game_over_snd;
Image court_img;
Image hoop_img;
Texture hoop;
Image ball_img;
Texture ball;

void interface_init() {
	// Αρχικοποίηση του παραθύρου
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "game");
	SetTargetFPS(60);
    InitAudioDevice();

	// Φόρτωση εικόνων και ήχων
	
	court_img = LoadImage("assets/court.png");
	ImageResize(&court_img, SCREEN_WIDTH, GROUND_LINE - 1);
	court = LoadTextureFromImage(court_img);

	hoop_img = LoadImage("assets/hoop7.png");
	ImageResize(&hoop_img, 100, SCREEN_HEIGHT - GROUND_LINE);
	hoop = LoadTextureFromImage(hoop_img);

	
	ball_img = LoadImage("assets/Basketball.png");
	ImageResize(&ball_img, 50, 50);
	ball = LoadTextureFromImage(ball_img);

	game_over_snd = LoadSound("assets/boo2.mp3");
}

void interface_close() {

	UnloadTexture(ball);
	UnloadImage(ball_img);

	UnloadTexture(hoop);
	UnloadImage(hoop_img);

	UnloadTexture(court);
	UnloadImage(court_img);

	CloseAudioDevice();
	CloseWindow();
}

// Draw game (one frame)
void interface_draw_frame(State state) {
	StateInfo info = state_info(state);
	BeginDrawing();

	// Καθαρισμός, θα τα σχεδιάσουμε όλα από την αρχή
	ClearBackground(RAYWHITE);
	DrawTexture(court, 0, 0, RAYWHITE);

	//Γραμμη πατωματος
	DrawLine(0, GROUND_LINE, SCREEN_WIDTH, GROUND_LINE, BLACK);

	//Χρωμα πατωματος
	DrawRectangle(0, GROUND_LINE + 1, SCREEN_WIDTH, SCREEN_HEIGHT, ORANGE);

	//Character
	DrawTexture(ball, CHAR_POS_X, GROUND_LINE + info->character->rect.y, RAYWHITE);
	//DrawRectangle(CHAR_POS_X, GROUND_LINE + info->character->rect.y, info->character->rect.width, info->character->rect.height, RED);
	
	List obj_to_draw = state_objects(state, X_FROM, X_TO);

	for(ListNode node = list_first(obj_to_draw) ; node != LIST_EOF ; node = list_next(obj_to_draw, node)){
		Object obj = list_node_value(obj_to_draw, node);
		if (obj->type == OBSTACLE){
			DrawRectangle(obj->rect.x - info->character->rect.x + CHAR_POS_X, GROUND_LINE - obj->rect.height, obj->rect.width, obj->rect.height, BLUE);
		}
		else if(obj->type == ENEMY){
			DrawRectangle(obj->rect.x - info->character->rect.x + CHAR_POS_X, GROUND_LINE - obj->rect.height, obj->rect.width, obj->rect.height, GREEN);
		}
		else{
			DrawTexture(hoop, obj->rect.x - info->character->rect.x + CHAR_POS_X, GROUND_LINE - obj->rect.height, RAYWHITE);
			//DrawRectangle(obj->rect.x - info->character->rect.x + CHAR_POS_X, GROUND_LINE - obj->rect.height, obj->rect.width, obj->rect.height, DARKPURPLE);
		}
		
	}
	//DrawTexture(Basketball, SCREEN_WIDTH/2 - info->character->rect.width, GROUND_LINE + info->character->rect.y - info->character->rect.height, RED);

	// Σχεδιάζουμε το σκορ και το FPS counter
	int score = (int)info->character->rect.x / 2800;

	if (score > PORTAL_NUM){
		score = PORTAL_NUM;
	}
	else if (score < 0){
		score = 0;
	}

	if (score < 50){
		DrawText(TextFormat("%03d", score), 20, 20, 40, BEIGE);
	}
	else if (score < 75){
		DrawText(TextFormat("%03d", score), 20, 20, 40, ORANGE);
	}
	else if (score < 90){
		DrawText(TextFormat("%03d", score), 20, 20, 40, LIME);
	}
	else{
		DrawText(TextFormat("%03d", score), 20, 20, 40, MAROON);
	}

	list_destroy(obj_to_draw);

	DrawText(TextFormat("%02d", info->wins), 20, 50, 40, DARKGREEN);
	DrawFPS(SCREEN_WIDTH - 80, 0);

	// Αν το παιχνίδι έχει τελειώσει, σχεδιάζομαι το μήνυμα για να ξαναρχίσει και παιζουμε το game_over sound
	if (!(info->playing)) {
		DrawText(
			"PRESS [ENTER] TO PLAY",
			 GetScreenWidth() / 2 - MeasureText("PRESS [ENTER] TO PLAY", 20) / 2,
			 GetScreenHeight() / 2 - 50, 20, GRAY
		);
		PlaySound(game_over_snd);
	}
	else if (info->paused) {			// Παυση
		DrawText(
			"PRESS P TO QUIT PAUSE OR N TO LOAD NEXT FRAME",
			 GetScreenWidth() / 2 - MeasureText("PRESS P TO QUIT PAUSE OR N TO LOAD NEXT FRAME", 20) / 2,
			 GetScreenHeight() / 2 - 50, 20, GRAY
		);
	}


	EndDrawing();
	
}