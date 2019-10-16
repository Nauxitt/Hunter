#ifndef __menubar_h
#define __menubar_h

#include "stateengine.h"
#include "hunter.h"

// typedef struct _MenubarState MenubarState;
// typedef struct _MatchMenubarState MissionMenuState;
// typedef struct _MainMenuMenubarState MainMenuMenubarState;

typedef struct _MenubarState {
	GameState state;
	SDL_Texture * background_texture;
	SDL_Texture * buttons_texture;
	MatchContext * match;
	int8_t length;
	int8_t selector;   // Index of the item being selected
	uint8_t active;    // Whether the menubar is selecting anything
} MenubarState;

#define MenubarState(M) ((MenubarState *) M)

MenubarState * initMenu(MenubarState * state, MatchContext * match);
void menuOnDraw(EventHandler * h);
void menuOnKeyUp(EventHandler * h, SDL_Event * e);

#endif
