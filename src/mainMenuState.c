#include <SDL2/SDL.h>
#include "stateengine.h"
#include "mainMenuState.h"

MainMenuState * initMainMenuState(MainMenuState * state){
	if(state == NULL)
		state = (MainMenuState*) calloc(sizeof(MainMenuState), 1);
	
	EventHandler(state)->onDraw = mainMenuOnDraw;

	return state;
}

void mainMenuOnKeyDown(EventHandler * h){}
void mainMenuOnDraw(EventHandler * h){}
