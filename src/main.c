#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#include <SDL2/SDL_image.h>

#include "main.h"
#include "animations.h"
#include "stateengine.h"
#include "mapstate.h"
#include "hunter.h"

Game game;

int main(){
	initGame();
	
	MapState * mapstate = makeMapState(NULL, 10, 10);
	
	// Generate a simple map
	Map * map = mapstate->map;
	for_xy(x, y, map->w, map->h){
		Tile * tile = getTile(map, x, y);
		if(		(x < 2) | (x >= map->w - 2) |
				(y < 2) | (y >= map->h - 2) )
			tile->val = 1;
	}

	Hunter daniel_character = {
		.name = "Daniel", .base_stats={.atk = 2, .mov = 3, .def = 6},
		.type = "hunter"
	};
	Hunter dave_character = {
		.name = "Dave", .base_stats = {.mov = 3, .atk = 4, .def = 4},
		.type = "hunter"
	};
	Hunter stan_character = {
		.name = "Stan", .base_stats = {.atk = 2, .mov = 1, .def = 8},
		.type = "hunter"
	};
	Hunter tim_character = {
		.name = "Tim", .base_stats = {.atk = 9, .mov = 1, .def = 1},
		.type = "hunter"
	};

	HunterEntity * daniel = initHunter(
			&mapstate->hunters[0], mapstate, mapstate->daniel_texture);
	HunterEntity * dave = initHunter(
			&mapstate->hunters[1], mapstate, mapstate->daniel_texture);
	HunterEntity * stan = initHunter(
			&mapstate->hunters[2], mapstate, mapstate->daniel_texture);
	HunterEntity * tim = initHunter(
			&mapstate->hunters[3], mapstate, mapstate->daniel_texture);

	daniel->hunter = &daniel_character;
	dave->hunter = &dave_character;
	stan->hunter = &stan_character;
	tim->hunter = &tim_character;

	mapstate->daniel = daniel;
	hunterSetTile(daniel, 1,1);
	hunterSetTile(dave,   6,0);
	hunterSetTile(stan,   0,4);
	hunterSetTile(tim,    8,7);

	Relic floppy = {.item_id=0, .name="floppy"};
	Relic book = {.item_id=1, .name="book"};
	Relic crystal = {.item_id=2, .name="crystal"};
	Relic metal = {.item_id=3, .name="metal"};

	Crate * crates = calloc(sizeof(Crate), 2);
	crates[0].x = 1;
	crates[0].y = 0;
	crates[0].contents = NULL;
	crates[0].exists = 1;
	crates[0].contents = &floppy;

	crates[1].x = 5;
	crates[1].y = 8;
	crates[1].contents = NULL;
	crates[1].exists = 1;
	crates[1].contents = &metal;

	MatchContext context = {
		.characters = {
			&daniel_character,
			&dave_character,
			&stan_character,
			&tim_character
		},
		.crates_len = 2,
		.crates = crates
	};

	CrateEntity * crate_entities = calloc(sizeof(CrateEntity), 2);
	SDL_Texture * crate_texture = IMG_LoadTexture(
			game.renderer, "resources/crate.png"
		);

	initCrateEntity(&crate_entities[0], mapstate, crate_texture);
	initCrateEntity(&crate_entities[1], mapstate, crate_texture);
	crate_entities[0].crate = &crates[0];
	crate_entities[1].crate = &crates[1];
	crateSetTile(&crate_entities[0], crates[0].x, crates[0].y);
	crateSetTile(&crate_entities[1], crates[1].x, crates[1].y);
	
	mapstate->match = &context;
	initMatch(&context);

	gamePushState(GameState(mapstate));
	gameMainLoop();

	return 0;
}
