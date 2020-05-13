#ifndef __mainMenuState_h
#define __mainMenuState_h

#include "stateengine.h"
#include "menubar.h"
#include "sprites.h"
#include "statboxDisplayState.h"
#include "brokerState.h"
#include "nurseState.h"
#include "loadHunterState.h"
#include "characterCreatorState.h"

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

	MenubarState menubar_main;
	MenubarState menubar_hunter;

	LoadHunterState load_hunter;

	StatboxDisplayState statbox;
	CharacterCreatorState character_creator;
	WallpaperTransitionState transition;
	int wallpaper;

	int hunter_selected;
	Hunter * hunters[4];
	Hunter hunter_memory[4];

	BrokerState broker;
	NurseState nurse;
} MainMenuState;

#define MainMenuState(M) ((MainMenuState*) M)

MainMenuState * initMainMenuState(MainMenuState * state);
void mainMenuOnEnter(EventHandler * h);
void mainMenuOnKeyUp(EventHandler * h, SDL_Event * e);
void mainMenuHunterMenubarOnSpace(MainMenuState * state);
void mainMenuOnDraw(EventHandler * h);

void mainMenuMainMenubarOnSpace(MainMenuState * state);

void mainMenuStartBasicMission(MainMenuState * menu);
void mainMenuTransitionOut(MainMenuState * state, int wallpaper);
void mainMenuTransitionIn(MainMenuState * state);

WallpaperTransitionState * makeWallpaperTransitionState(WallpaperTransitionState * state, int top, int bottom);
void wallpaperTransitionStateOnDraw(EventHandler * h);

#endif
