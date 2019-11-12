#ifndef __mainMenuState_h
#define __mainMenuState_h

#include "stateengine.h"
#include "menubar.h"
#include "sprites.h"
#include "statboxDisplayState.h"
#include "brokerState.h"

typedef struct _WallpaperTransitionState {
	GameState state;
	MenubarState * menubar;
	StatboxDisplayState * statbox;

	int top;
	int bottom;
	int npc;
	int reverse;
	
	int duration;
} WallpaperTransitionState;

typedef struct _MainMenuState {
	GameState state;
	MenubarState * menubar;
	StatboxDisplayState statbox;
	WallpaperTransitionState transition;
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
void mainMenuTransitionOut(MainMenuState * state, int wallpaper);
void mainMenuTransitionIn(MainMenuState * state);

WallpaperTransitionState * makeWallpaperTransitionState(WallpaperTransitionState * state, int top, int bottom);
void wallpaperTransitionStateOnDraw(EventHandler * h);

#endif
