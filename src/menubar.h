#ifndef __menubar_h
#define __menubar_h

#include "stateengine.h"
#include "hunter.h"

#define MENUBAR_MAX_ICONS 20    // arbitrary number

typedef struct _MenubarIcon {
	int id;
	// on highlight
	// on activate
} MenubarIcon;

typedef struct _MenubarState {
	GameState state;
	SDL_Texture  * background_texture;
	SDL_Texture  * buttons_texture;
	MatchContext * match;
	int8_t length;
	int8_t selector;   // Index of the item being selected
	uint8_t active;    // Whether the menubar is selecting anything
	MenubarIcon icons[MENUBAR_MAX_ICONS];
} MenubarState;

#define MenubarState(M) ((MenubarState *) M)

MenubarState * initMenu(MenubarState * state, MatchContext * match);
void menuOnDraw(EventHandler * h);
void menuOnKeyUp(EventHandler * h, SDL_Event * e);

void drawMenubarIcon(int x, int y, int id);
void drawMenubarBackground(SDL_Rect * dest);

#endif
