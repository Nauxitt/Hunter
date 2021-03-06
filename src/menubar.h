#ifndef __menubar_h
#define __menubar_h

#include "stateengine.h"
#include "hunter.h"

#define MENUBAR_MAX_ICONS 6    // arbitrary number

typedef struct _MenubarIcon {
	int id;
	char * help_text;
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
	uint32_t selector_change_time;
	int scroll_speed;
	void (*drawContents)(struct _MenubarState * menu);
} MenubarState;

#define MenubarState(M) ((MenubarState *) M)

MenubarState * initMenu(MenubarState * state);
void menuOnDraw(EventHandler * h);
void menuOnKeyUp(EventHandler * h, SDL_Event * e);

void drawMenubarIcon(int x, int y, int id);
void drawMenubarBackground(SDL_Rect * dest);
void drawMenubarContents(MenubarState * menu);

#endif
