#ifndef __mainMenuState_h
#define __mainMenuState_h

#include "stateengine.h"
#include "menubar.h"
#include "sprites.h"

typedef struct _MainMenuState {
	GameState state;
	MenubarState * menubar;
	int wallpaper;
	int hunter_selected;
	Hunter * hunters[4];
} MainMenuState;

typedef struct _BrokerState {
	GameState state;
	MenubarState * menubar;
} BrokerState;

typedef struct _NurseState {
	GameState state;
	MenubarState * menubar;
} NurseState;

#define MainMenuState(M) ((MainMenuState*) M)

MainMenuState * initMainMenuState(MainMenuState * state);
void mainMenuOnKeyUp(EventHandler * h, SDL_Event * e);
void mainMenuOnDraw(EventHandler * h);

#endif
