#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>

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
	};
	Hunter dave_character = {
		.name = "Dave", .base_stats = {.mov = 3, .atk = 4, .def = 4},
	};
	Hunter stan_character = {
		.name = "Stan", .base_stats = {.atk = 2, .mov = 1, .def = 8},
	};
	Hunter tim_character = {
		.name = "Tim", .base_stats = {.atk = 9, .mov = 1, .def = 1},
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

	MatchContext context = {
		.characters = {
			&daniel_character,
			&dave_character,
			&stan_character,
			&tim_character
		},
	};
	
	mapstate->match = &context;
	initMatch(&context);

	gamePushState(GameState(mapstate));
	gameMainLoop();

	return 0;
}