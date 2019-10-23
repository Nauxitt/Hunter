#include <SDL2/SDL.h>
#include "stateengine.h"
#include "mainMenuState.h"
#include "menubar.h"
#include "draw.h"
#include "sprites.h"

MainMenuState * initMainMenuState(MainMenuState * state){
	if(state == NULL)
		state = (MainMenuState*) calloc(sizeof(MainMenuState), 1);
	
	state->menubar = initMenu(NULL, NULL);
	EventHandler(state)->onDraw = mainMenuOnDraw;
	EventHandler(state)->onKeyDown = mainMenuOnKeyDown;
	state->menubar->drawContents = drawMenubarContents;

	return state;
}

void mainMenuOnKeyDown(EventHandler * h, SDL_Event * e){
	MainMenuState * state = MainMenuState(h);

	switch(e->key.keysym.scancode){
		case SDL_SCANCODE_TAB:
			state->wallpaper++;
			if(state->wallpaper >= textures.wallpapers.tiles_num)
				state->wallpaper = 0;
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
