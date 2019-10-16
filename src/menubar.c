#include "menubar.h"
#include "stateengine.h"
#include "hunter.h"
#include "sprites.h"
#include "draw.h"

MenubarState * initMenu(MenubarState * state, MatchContext * match){
	if(state == NULL)
		state = MenubarState(calloc(sizeof(MenubarState), 1));

	state->match = match;
	state->drawContents = drawMenubarContents;

	// Hard-code mission menubar data
	state->drawContents = matchMenubarDrawContents;
	state->length = 5;
	state->icons[0].id = 0;
	state->icons[1].id = 1;
	state->icons[2].id = 2;
	state->icons[3].id = 3;
	state->icons[4].id = 4;
	state->icons[5].id = -1;

	EventHandler(state)->type = "MenubarState";
	EventHandler(state)->onDraw = menuOnDraw;
	EventHandler(state)->onKeyUp = menuOnKeyUp;

	return state;
}

void menuOnKeyUp(EventHandler * h, SDL_Event * e){
	MenubarState * state = MenubarState(h);

	switch(e->key.keysym.scancode){
		case SDL_SCANCODE_LEFT:
			if(--state->selector < 0)
				state->selector = state->length-1;
			break;

		case SDL_SCANCODE_RIGHT:
			if(++state->selector >= state->length)
				state->selector = 0;
			break;

		default:
			break;
	}
}

void drawMenubarBackground(SDL_Rect * dest){
	SDL_Rect src = {0, 0, 1, 64};
	blit(textures.menu_gradient.texture, &src, dest);
}

void drawMenubarIcon(int x, int y, int id){
	spritesheetBlit(&textures.menu_icons, id,0, x,y);
}

void drawMenubarContents(MenubarState * menu){
	// Render left icons
	for(int i=0; i < menu->length; i++){
		MenubarIcon * icon = &menu->icons[i];
		int id = icon->id;

		if(id < 0){
			menu->length = i;
			break;
		}

		drawMenubarIcon(id*38 + 32, 24, id);
	}
}

void matchMenubarDrawContents(MenubarState * menu){
	MatchContext * match = menu->match;

	drawMenubarContents(menu);
	
	// Draw scroling text window
	SDL_Rect src = {0, 16, 144, 16};

	SDL_Rect dest = {5*38 + 32, 24, 144*2, 32};
	blit(textures.menu_icons.texture, &src, &dest);

	drawDeckIndicator(game.w - 32 * 4, 24, match->deck_len);
}

void menuOnDraw(EventHandler * h){
	MenubarState * menu = MenubarState(h);

	// Render menubar background
	SDL_Rect dest = {0, 0, game.w, 64};
	drawMenubarBackground(&dest);

	if(menu->drawContents)
		menu->drawContents(menu);

	// Draw selector feather
	// TODO: generalize outside of matches (get rid of polling the action)
	if(pollAction("poll_turn_action")){
		SDL_Rect src = {0, 32, 32, 32};
		
		if(menu->selector != -1){
			dest.x = menu->selector * 38 + 32;
			dest.y = 24;
			dest.w = 64; dest.h = 64;
			blit(textures.menu_icons.texture, &src, &dest);
		}
	}
}
