#include "menubar.h"
#include "stateengine.h"
#include "hunter.h"
#include "sprites.h"
#include "draw.h"

MenubarState * initMenu(MenubarState * state, MatchContext * match){
	if(state == NULL)
		state = MenubarState(calloc(sizeof(MenubarState), 1));

	state->match = match;

	// Hard-code mission icon data
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


void menuOnDraw(EventHandler * h){
	MenubarState * menu = MenubarState(h);
	MatchContext * match = menu->match;

	// Render menubar background
	SDL_Rect dest = {0, 0, game.w, 64};
	
	drawMenubarBackground(&dest);
	
	// Render left icons
	dest.y = 24;

	SDL_Rect src = {0, 0, 16, 16};
	for(int i=0; i < menu->length; i++){
		MenubarIcon * icon = &menu->icons[i];
		int id = icon->id;

		if(id < 0){
			menu->length = i;
			break;
		}

		drawMenubarIcon(id*38 + 32, dest.y, id);
	}

	// Draw scroling text window
	src.x = 0;    src.y = 16;
	src.w = 144;  src.h = 16;

	dest.x = 5 * 38 + 32;
	dest.w = 144*2;  dest.h = 32;
	blit(textures.menu_icons.texture, &src, &dest);

	drawDeckIndicator(game.w - 32 * 4, 24, match->deck_len);

	// Draw selector feather
	if(pollAction("poll_turn_action")){
		src.x = 0; src.y = 32;
		src.w = 32; src.h = 32;
		
		if(menu->selector != -1){
			dest.x = menu->selector * 38 + 32;
			dest.w = 64; dest.h = 64;
			blit(textures.menu_icons.texture, &src, &dest);
		}
	}
}
