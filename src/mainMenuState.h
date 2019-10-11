#ifndef __mainMenuState_h
#define __mainMenuState_h

#include "stateengine.h"

typedef struct _MainMenuState {
	GameState state;

	// Menubar
	// Background
	// Screen contents state?
} MainMenuState;

MainMenuState * initMainMenuState(MainMenuState * state);
void mainMenuOnKeyDown(EventHandler * h);
void mainMenuOnDraw(EventHandler * h);

#endif
