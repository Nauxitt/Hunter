#include <SDL2/SDL.h>
#include "stateengine.h"
#include "mainMenuState.h"
#include "menubar.h"
#include "sprites.h"

MainMenuState * initMainMenuState(MainMenuState * state){
	if(state == NULL)
		state = (MainMenuState*) calloc(sizeof(MainMenuState), 1);
	
	state->menubar = initMenu(NULL, NULL);
	EventHandler(state)->onDraw = mainMenuOnDraw;
	state->menubar->drawContents = drawMenubarContents;

	return state;
}

void mainMenuOnKeyDown(EventHandler * h){}

void mainMenuOnDraw(EventHandler * h){
	MainMenuState * state = MainMenuState(h);
	onDraw(EventHandler(state->menubar));
}
