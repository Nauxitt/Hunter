#include <SDL2/SDL.h>
#include "stateengine.h"
#include "mainMenuState.h"
#include "menubar.h"
#include "draw.h"
#include "sprites.h"

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
					// TODO: Instead of going into submenu, creates a new random hunter.
					break;

				case 1: // Broker
					// TODO: Instead of visiting the man, the legend himself in his office, initializes a new match
					break;

				case 2: // Nurse
					// TODO: Instead of I Heal You, levels your hunter up.
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
			// TODO: cycles active hunter slot
			break;

		default:
			break;
	}
}

void mainMenuOnDraw(EventHandler * h){
	MainMenuState * state = MainMenuState(h);
	drawWallpaper(state->wallpaper);
	onDraw(EventHandler(state->menubar));
}
