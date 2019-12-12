#include <SDL2/SDL.h>
#include "stateengine.h"
#include "mainMenuState.h"
#include "menubar.h"
#include "draw.h"
#include "sprites.h"
#include "hunter.h"
#include "mapstate.h"
#include "userdata.h"

#include "brokerState.h"
#include "nurseState.h"

MainMenuState * initMainMenuState(MainMenuState * state){
	if(state == NULL)
		state = (MainMenuState*) calloc(sizeof(MainMenuState), 1);
	
	EventHandler(state)->onEnter = mainMenuOnEnter;
	EventHandler(state)->onDraw = mainMenuOnDraw;
	EventHandler(state)->onKeyUp = mainMenuOnKeyUp;

	initMenu(&state->menubar_main, NULL);
	state->menubar_main.drawContents = drawMenubarContents;
	state->menubar_main.selector = 0;
	state->menubar_main.active = 1;
	state->menubar_main.length = 4;
	state->menubar_main.icons[0].id = 14;
	state->menubar_main.icons[1].id = 15;
	state->menubar_main.icons[2].id = 16;
	state->menubar_main.icons[3].id =  4;
	state->menubar_main.icons[4].id = -1;

	initMenu(&state->menubar_hunter, NULL);
	state->menubar_hunter.drawContents = drawMenubarContents;
	state->menubar_hunter.selector = 0;
	state->menubar_hunter.active = 1;
	state->menubar_hunter.length = 5;
	state->menubar_hunter.icons[0].id = 18;
	state->menubar_hunter.icons[1].id = 19;
	state->menubar_hunter.icons[2].id = 20;
	state->menubar_hunter.icons[3].id = 21;
	state->menubar_hunter.icons[4].id = 22;
	state->menubar_hunter.icons[5].id = -1;

	state->menubar = &state->menubar_main;

	makeStatboxDisplayState(&state->statbox);
	state->statbox.hunters_list = (Hunter**) &state->hunters;

	makeBrokerState(&state->broker);
	makeNurseState(&state->nurse);
	state->broker.statbox = &state->statbox;
	state->nurse.statbox = &state->statbox;

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

void mainMenuHunterMenubarOnSpace(MainMenuState * state){
	switch (state->menubar->selector) {
		case 0: // Make new hunter
			state->hunters[state->hunter_selected] = randomHunter(state->hunters[state->hunter_selected], 0);
			state->hunters[state->hunter_selected]->id = state->hunter_selected;
			gamePushState((GameState*) makeStatAllocatorState(
					&state->allocator, state->hunters[state->hunter_selected], 10
				));
			state->allocator.color = state->hunter_selected;
			break;

		case 1:  // Save hunter
			if(state->hunters[state->hunter_selected])
				hunterSave(state->hunters[state->hunter_selected]);
			break;

		case 2:
			gamePushState( makeLoadHunterState(
						&state->load_hunter,
						state->hunters[state->hunter_selected]
					));
			state->hunters[state->hunter_selected] = state->load_hunter.hunter;
			break;

		case 3:  // TODO: Hunter stats
			break;

		case 4:  // TODO: Remove hunter
			break;
	}
}

void mainMenuMainMenubarOnSpace(MainMenuState * state){
	switch(state->menubar->selector){
		case 0: // Hunter options
			// Enter Hunter submenubar
			state->menubar = &state->menubar_hunter;
			state->menubar->selector = 0;
			break;

		case 1: // Broker
			gamePushState((GameState*) &state->broker);
			mainMenuTransitionOut(state, 22);
			state->transition.menubar = state->menubar;
			state->transition.statbox = &state->statbox;
			state->transition.npc = 0;
			break;

		case 2: // Nurse
			// Instead of I Heal You, levels your hunter up

			gamePushState((GameState*) &state->nurse);
			state->transition.menubar = state->menubar;
			state->transition.statbox = &state->statbox;
			mainMenuTransitionOut(state, 23);
			state->transition.npc = 1;

			break;

		case 3: // Options
			// Currently, instead of changing a plenthora of settings, cycles the wallpaper, all of which you've unlocked.
			state->wallpaper++;
			if(state->wallpaper >= textures.wallpapers.tiles_num)
				state->wallpaper = 0;
			break;
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
			if(state->menubar == &state->menubar_main)
				mainMenuMainMenubarOnSpace(state);
			else if(state->menubar == &state->menubar_hunter)
				mainMenuHunterMenubarOnSpace(state);
			break;

		// Escape key exits hunter submenubar
		case SDL_SCANCODE_ESCAPE:
			if (state->menubar == &state->menubar_hunter) {
				state->menubar = &state->menubar_main;
				state->menubar->selector = 0;
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

	state->statbox.selector = state->hunter_selected;
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
				state->npc,0,
				game.w-(textures.character_portraits.w*time)/state->duration,
				game.h-textures.character_portraits.h
			);

	drawWallpaperTransition(state->top, time * game.w/2 / state->duration);

	if(state->menubar)
		onDraw(EventHandler(state->menubar));

	if(state->statbox)
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
