#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#include <SDL2/SDL_image.h>

#include "main.h"
#include "animations.h"
#include "stateengine.h"
#include "mapstate.h"
#include "hunter.h"
#include "sprites.h"

Game game;

int main(){
	initGame();
	loadSprites();

	Hunter hunters[] = {
		{	.name = "Daniel",
			.base_stats={.atk = 2, .mov = 3, .def = 6},
			.type = "hunter"
		},
		{	.name = "Dave",
			.base_stats = {.mov = 3, .atk = 4, .def = 4},
			.type = "hunter"
		},
		{	.name = "Stan",
			.base_stats = {.atk = 2, .mov = 1, .def = 8},
			.type = "hunter"
		},
		{	.name = "Tim",
			.base_stats = {.atk = 9, .mov = 1, .def = 1},
			.type = "hunter"
		}
	};

	Relic floppy = {.item_id=0, .name="floppy"};
	// Relic book = {.item_id=1, .name="book"};
	// Relic crystal = {.item_id=2, .name="crystal"};
	Relic metal = {.item_id=3, .name="metal"};

	Crate * crates = calloc(sizeof(Crate), 2);
	crates[0].contents = NULL;
	crates[0].exists = 1;
	crates[0].contents = &floppy;

	crates[1].contents = NULL;
	crates[1].exists = 1;
	crates[1].contents = &metal;

	
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

	char * map_encoded = 
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
		"  ### # ###\n";

	// Calculate map diminsions
	int x=0, y=0, w=0, h=0;
	for(char * c = map_encoded; *c; c++){
		if(*c == '\n'){
			if(x > w) w = x;
			y++;
			x = -1;
		}
		x++;
	}
	h = y;

	Tile * map = (Tile*) calloc(sizeof(Tile), w*h);

	// Decode map contents
	Hunter * hunter_current = (Hunter*) &hunters;
	Crate * crate_current = crates;
	int exit_x, exit_y;

	x = 0;
	y = 0;
	for(char * c = map_encoded; *c; c++){
		Tile * tile = map + w*y + x;
		tile->exists = 1;

		switch(*c){
			case ' ':
				tile->exists = 0;
				break;

			case '#':
				break;

			case 'E':
				exit_x = x;
				exit_y = y;
				break;

			case 'H':
				hunter_current->x = x;
				hunter_current->y = y;
				tile->hunter = hunter_current++;
				break;

			case 'C':
				crate_current->x = x;
				crate_current->y = y;
				tile->crate = crate_current++;
				break;

			case '\n':
				y++;
				x = -1;
				break;
		}
		x++;
	}

	MatchContext context = {
		.characters = {
			&hunters[0], &hunters[1],
			&hunters[2], &hunters[3],
		},
		.crates_len = 2,
		.crates = crates,
		.exit_x = exit_x,
		.exit_y = exit_y,
		.map_w = w,
		.map_h = h,
		.map = map
	};

	// Make mapstate

	MapState * mapstate = makeMapState(NULL, w, h);
	for(int n=0; n < w*h; n++){
		mapstate->map->tiles[n].val = map[n].exists;
	}

	for(int n=0; n < 4; n++){
		initHunter(&mapstate->hunters[n], mapstate, textures.daniel.texture);
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
	
	mapstate->match = &context;
	initMatch(&context);

	gamePushState(GameState(mapstate));
	gameMainLoop();

	return 0;
}
