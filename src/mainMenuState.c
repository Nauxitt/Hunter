#include <SDL2/SDL.h>
#include "stateengine.h"
#include "mainMenuState.h"
#include "menubar.h"
#include "draw.h"
#include "sprites.h"
#include "hunter.h"
#include "mapstate.h"

MainMenuState * initMainMenuState(MainMenuState * state){
	if(state == NULL)
		state = (MainMenuState*) calloc(sizeof(MainMenuState), 1);
	
	EventHandler(state)->onEnter = mainMenuOnEnter;
	EventHandler(state)->onDraw = mainMenuOnDraw;
	EventHandler(state)->onKeyUp = mainMenuOnKeyUp;

	state->menubar = initMenu(NULL, NULL);
	state->menubar->drawContents = drawMenubarContents;
	state->menubar->selector = 0;
	state->menubar->active = 1;
	state->menubar->length = 4;
	state->menubar->icons[0].id = 14;
	state->menubar->icons[1].id = 15;
	state->menubar->icons[2].id = 16;
	state->menubar->icons[3].id =  4;
	state->menubar->icons[4].id = -1;

	makeStatboxDisplayState(&state->statbox);
	state->statbox.hunters_list = (Hunter**) &state->hunters;

	makeBrokerState(&state->broker);
	state->broker.statbox = &state->statbox;

	return state;
}

void mainMenuOnEnter(EventHandler * h){
	MainMenuState * state = (MainMenuState *) h;

	for(int h=0; h < 4; h++){
		Hunter * hunter = state->hunters[h];

		if(hunter == NULL)
			continue;

		// Clear hunter's hand
		for(int n=0; n < HAND_LIMIT; n++)
			hunter->hand[n] = NULL;

		// Mark player's items as owned
		for(int n=0; n < INVENTORY_LIMIT; n++){
			Relic * relic = hunter->inventory[n];
		
			if(relic == NULL)
				continue;

			relic->player_item = 1;
		}
	}
}


void mainMenuOnKeyUp(EventHandler * h, SDL_Event * e){
	MainMenuState * state = MainMenuState(h);

	switch(e->key.keysym.scancode){
		// Return cycles hunter
		case SDL_SCANCODE_RETURN:
			state->hunter_selected++;
			if(state->hunter_selected >= 4)
				state->hunter_selected = 0;
			break;

		// Space operates menubar
		case SDL_SCANCODE_SPACE:
			switch(state->menubar->selector){
				case 0: // Hunter options
					// Instead of going into submenu, creates a new hunter.
					state->hunters[state->hunter_selected] = randomHunter(state->hunters[state->hunter_selected], 0);
					gamePushState((GameState*) makeStatAllocatorState(
							&state->allocator, state->hunters[state->hunter_selected])
						);
					state->allocator.rect.x = 16;
					state->allocator.rect.y = 64 + 16;
					break;

				case 1: // Broker
					gamePushState((GameState*) &state->broker);
					mainMenuTransitionOut(state, 22);
					state->transition.menubar = state->menubar;
					state->transition.statbox = &state->statbox;
					state->transition.npc = 0;
					break;

				case 2: // Nurse
					// Instead of I Heal You, levels your hunter up with a random stat.
					if(state->hunters[state->hunter_selected])
						hunterRandomStatIncrease(state->hunters[state->hunter_selected], 1);
					break;

				case 3: // Options
					// Currently, instead of changing a plenthora of settings, cycles the wallpaper, all of which you've unlocked.
					state->wallpaper++;
					if(state->wallpaper >= textures.wallpapers.tiles_num)
						state->wallpaper = 0;
					break;
			}
			break;

		case SDL_SCANCODE_LEFT:
		case SDL_SCANCODE_RIGHT:
			// Forward event to menubar
			onKeyUp((EventHandler*) state->menubar, e);
			break;

		case SDL_SCANCODE_TAB:
			onKeyUp((EventHandler*) &state->statbox, e);
			break;

		default:
			break;
	}
}

void mainMenuOnDraw(EventHandler * h){
	MainMenuState * state = MainMenuState(h);
	drawWallpaper(state->wallpaper);
	onDraw(EventHandler(state->menubar));
	onDraw(EventHandler(&state->statbox));
}

void mainMenuTransitionOut(MainMenuState * state, int wallpaper){
	WallpaperTransitionState * transition = &state->transition;
	makeWallpaperTransitionState(transition, state->wallpaper, wallpaper);
	gamePushState(GameState(transition));
}

void mainMenuTransitionIn(MainMenuState * state){
	WallpaperTransitionState * transition = &state->transition;
	transition->reverse = 1;
	gamePushState(GameState(transition));
}

WallpaperTransitionState * makeWallpaperTransitionState(WallpaperTransitionState * state, int top, int bottom){
	if(state == NULL)
		state = (WallpaperTransitionState*) calloc(sizeof(WallpaperTransitionState), 1);
	
	EventHandler(state)->type = "WallpaperTransitionState";
	EventHandler(state)->onDraw = wallpaperTransitionStateOnDraw;

	state->top = top;
	state->bottom = bottom;
	state->duration = 250;
	state->npc = -1;
	state->reverse = 0;

	return state;
}

void wallpaperTransitionStateOnDraw(EventHandler * h){
	WallpaperTransitionState * state = (WallpaperTransitionState*) h;
	uint32_t time = GameState(state)->duration;

	if(state->reverse)
		time = state->duration - time;

	drawWallpaper(state->bottom);

	if(state->npc >= 0)
		spritesheetBlit(
				&textures.character_portraits,
				0,0,
				game.w-(textures.character_portraits.w*time)/state->duration,
				game.h-textures.character_portraits.h
			);

	drawWallpaperTransition(state->top, time * game.w/2 / state->duration);

	onDraw(EventHandler(state->menubar));
	onDraw(EventHandler(state->statbox));

	if(GameState(h)->duration >= ((WallpaperTransitionState*) h)->duration)
		gamePopState();
}

void mainMenuStartBasicMission(MainMenuState * state){
	MatchContext * match = (MatchContext *) gameCalloc(sizeof(MatchContext), 1);

	Relic * relics = (Relic *) calloc(sizeof(Relic), 2);
	relics[0].item_id = 0;
	strcpy(relics[0].name, "floppy");

	relics[1].item_id = 3;
	strcpy(relics[1].name, "metal");

	Crate * crates = (Crate *) gameCalloc(sizeof(Crate), 2);
	crates[0].exists = 1;
	crates[0].contents = &relics[0];
	crates[1].exists = 1;
	crates[1].contents = &relics[1];
	match->crates = crates;
	match->crates_len = 2;

	for(int n=0; n<4; n++)
		match->characters[n] = state->hunters[n];

	match->target_relic = &relics[0];
	match->scoring_context = &DEFAULT_SCORING_CONTEXT;

	decodeMap(match,
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

	initMatch(match);

	// Make mapstate - Tiles
	MapState * mapstate = makeMapState(NULL, match);
	for(int n=0; n < match->map_w * match->map_h; n++){
		mapstate->map->tiles[n].val = match->map[n].exists;
	}

	// Make mapstate - Hunter Entities
	for(int n=0; n < 4; n++){
		initHunterEntity(&mapstate->hunters[n], mapstate, textures.daniel.texture);
		mapstate->hunters[n].hunter = match->characters[n];
		hunterSetTile(&mapstate->hunters[n], match->characters[n]->x, match->characters[n]->y);
	}

	// Make mapstate - Crates
	CrateEntity * crate_entities = gameCalloc(sizeof(CrateEntity), 2);
	SDL_Texture * crate_texture = textures.crate.texture;

	initCrateEntity(&crate_entities[0], mapstate, crate_texture);
	initCrateEntity(&crate_entities[1], mapstate, crate_texture);
	crate_entities[0].crate = &crates[0];
	crate_entities[1].crate = &crates[1];
	crateSetTile(&crate_entities[0], crates[0].x, crates[0].y);
	crateSetTile(&crate_entities[1], crates[1].x, crates[1].y);
	mapstate->crates = crate_entities;
	mapstate->crates_len = 2;
	
	gamePushState(GameState(mapstate));
}
