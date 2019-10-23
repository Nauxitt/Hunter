#include <SDL2/SDL.h>
#include "stateengine.h"
#include "mainMenuState.h"
#include "menubar.h"
#include "draw.h"
#include "sprites.h"
#include "hunter.h"

MainMenuState * initMainMenuState(MainMenuState * state){
	if(state == NULL)
		state = (MainMenuState*) calloc(sizeof(MainMenuState), 1);
	
	EventHandler(state)->onDraw = mainMenuOnDraw;
	EventHandler(state)->onKeyUp = mainMenuOnKeyUp;

	state->menubar = initMenu(NULL, NULL);
	state->menubar->drawContents = drawMenubarContents;
	state->menubar->selector = 0;
	state->menubar->active = 1;
	state->menubar->length = 4;
	state->menubar->icons[4].id = -1;

	return state;
}

void mainMenuOnKeyUp(EventHandler * h, SDL_Event * e){
	MainMenuState * state = MainMenuState(h);

	switch(e->key.keysym.scancode){
		case SDL_SCANCODE_RETURN:
		case SDL_SCANCODE_SPACE:
			switch(state->menubar->selector){
				case 0: // Hunter options
					// Instead of going into submenu, creates a new random hunter.
					state->hunters[state->hunter_selected] = randomHunter(state->hunters[state->hunter_selected], 10);
					break;

				case 1: // Broker
					// TODO: Instead of visiting the man, the legend himself in his office, initializes a new match
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
			onKeyUp(EventHandler(state->menubar), e);
			break;

		case SDL_SCANCODE_TAB:
			state->hunter_selected++;
			if(state->hunter_selected >= 4)
				state->hunter_selected = 0;
			break;

		default:
			break;
	}
}

void mainMenuOnDraw(EventHandler * h){
	MainMenuState * state = MainMenuState(h);
	drawWallpaper(state->wallpaper);
	onDraw(EventHandler(state->menubar));
	
	// Draw Hunter statboxes
	int panel_gutter = 4;
	int panel_w = (game.w - 16*2 - 4*3) / 4;
	for(int h=0; h < 4; h++){
		Hunter * hunter = state->hunters[h];

		if(hunter == NULL)
			continue;

		drawStatbox(
				hunter,
				(enum StatboxViews) 0,
				(enum WindowColor) h,
				16 + (panel_w+panel_gutter)*h,
				game.h-160-panel_gutter
			);
	}
}
