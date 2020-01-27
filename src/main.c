#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#include <SDL2/SDL_image.h>
#include "stateengine.h"
#include "main.h"

// For mission
#include "animations.h"
#include "mapstate.h"
#include "hunter.h"
#include "sprites.h"
#include "score.h"

#include "ai.h"

// For main menu
#include "mainMenuState.h"

Game game;

int basicMission(){
	initGame();
	game.tick_delay = 1;
	loadSprites();

	Hunter hunters[] = {
		{	.name = "Daniel",
			.base_stats={.atk = 7, .mov = 6, .def = 3, .max_hp=1},
			.level = 1,
			.type = "hunter"
		},
		{	.name = "Dave",
			.base_stats = {.mov = 1, .atk = 11, .def = 1, .max_hp=1},
			.level = 1,
			.type = "hunter"
		},
		{	.name = "Stan",
			.base_stats = {.atk = 2, .mov = 3, .def = 8, .max_hp=1},
			.level = 1,
			.type = "hunter"
		},
		{	.name = "Tim",
			.base_stats = {.atk = 11, .mov = 1, .def = 1, .max_hp=1},
			.level = 1,
			.type = "hunter"
		}
	};

	Bot * bot = calloc(sizeof(Bot), 1);
	for (int n=0; n < 4; n++) {
		Hunter * hunter = &hunters[n];
		hunter->controller_data = bot;
		hunter->controller_hook = botControllerHook;
	}


	Relic floppy = {.item_id=0, .name="floppy"};
	// Relic book = {.item_id=1, .name="book"};
	// Relic crystal = {.item_id=2, .name="crystal"};
	Relic metal = {.item_id=3, .name="metal"};

	Crate * crates = (Crate*) calloc(sizeof(Crate), 2);
	crates[0].exists = 1;
	crates[0].contents = &floppy;

	crates[1].exists = 1;
	crates[1].contents = &metal;

	MatchContext context = {
		.characters = {
			&hunters[0], &hunters[1],
			&hunters[2], &hunters[3],
		},
		.crates_len = 2,
		.crates = crates,
		.target_relic = &floppy,
		.scoring_context = &DEFAULT_SCORING_CONTEXT
	};
	
	/*
		Generate a map from a string encoding
		Key:
			space | Empty
			\n    | Map row end
			\0    | Map termination
			#     | Map tile
			C     | Crate
			H     | Hunter
			E     | Exit
	*/

	decodeMap(&context,
			"   #C### ##\n"
			"# #H########\n"
			"#####     ##\n"
			" ##       ##\n"
			" ####     ##\n"
			"  #E#    #H#####\n"
			" ###     ###  ##\n"
			" ##H      ######\n"
			" ### ### ###\n"
			" #####C#H###\n"
			"  ### # ###\n"
		);

	initMatch(&context);

	// Make mapstate

	MapState * mapstate = makeMapState(NULL, &context);

	for(int n=0; n < 4; n++){
		initHunterEntity(&mapstate->hunters[n], mapstate, textures.daniel.texture);
		mapstate->hunters[n].hunter = &hunters[n];
		hunterSetTile(&mapstate->hunters[n], hunters[n].x, hunters[n].y);
	}

	CrateEntity * crate_entities = calloc(sizeof(CrateEntity), 2);
	SDL_Texture * crate_texture = textures.crate.texture;

	initCrateEntity(&crate_entities[0], mapstate, crate_texture);
	initCrateEntity(&crate_entities[1], mapstate, crate_texture);
	crate_entities[0].crate = &crates[0];
	crate_entities[1].crate = &crates[1];
	crateSetTile(&crate_entities[0], crates[0].x, crates[0].y);
	crateSetTile(&crate_entities[1], crates[1].x, crates[1].y);

	gamePushState(GameState(mapstate));
	gameMainLoop();

	return 0;
}

int developmentMainMenu(){
	initGame();
	loadSprites();

	MainMenuState * state = initMainMenuState(NULL);
	gamePushState((GameState*) state);

	gameMainLoop();

	return 0;
}

#include "path.h"
int main(int argc, char ** argv) {
	if (argc <= 1)
		return developmentMainMenu();
	
	// Debug launch modes

	char * mode = argv[1];
	if (strcmp(mode, "headless") == 0)
		return botMain();

	if (strcmp(mode, "basic") == 0)
		return basicMission();

	printf("Launch mode not recognized: %s\n", mode);
	return 1;

	/*
	return pathfindingMain();
	return usermain();
	*/
}
