#ifndef __mainMenuState_h
#define __mainMenuState_h

#include "stateengine.h"
#include "menubar.h"
#include "sprites.h"
#include "brokerState.h"

typedef struct _MainMenuState {
	GameState state;
	MenubarState * menubar;
	int wallpaper;
	int hunter_selected;
	Hunter * hunters[4];
	BrokerState broker;
} MainMenuState;

#define MainMenuState(M) ((MainMenuState*) M)

MainMenuState * initMainMenuState(MainMenuState * state);
void mainMenuOnEnter(EventHandler * h);
void mainMenuOnKeyUp(EventHandler * h, SDL_Event * e);
void mainMenuOnDraw(EventHandler * h);

void mainMenuStartBasicMission(MainMenuState * menu);

#endif
