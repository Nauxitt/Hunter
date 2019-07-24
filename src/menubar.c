#include "menubar.h"
#include "stateengine.h"
#include "hunter.h"
#include "sprites.h"
#include "draw.h"

void menuOnDraw(EventHandler * h){
	MenubarState * menu = MenubarState(h);
	MatchContext * match = menu->match;

	// Render menubar background
	SDL_Rect src = {0, 0, 1, 64};
	SDL_Rect dest = {0, 0, game.w, 64};
	
	blit(textures.menu_gradient.texture, &src, &dest);
	
	// Render left icons
	src.w  = 16; src.h  = 16;
	dest.y = 64 - 32 - 8;
	dest.w = 32; dest.h = 32;

	for(int i=0; i<5; i++){
		src.x = i * 16;
		dest.x = i * 38 + 32;
		spritesheetBlit(&textures.menu_icons, i,0, dest.x, dest.y);
	}

	// Draw deck icon
	src.x = 5 * 16;
	dest.x = game.w - 32 * 4;
	spritesheetBlit(&textures.menu_icons, 5,0, dest.x, dest.y);

	// Draw scroling text window
	src.x = 0; src.y = 16;
	src.w = 144;
	src.h = 16;
	dest.x = 5 * 38 + 32;
	dest.w = 144*2;
	blit(textures.menu_icons.texture, &src, &dest);

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

	// Draw deck card count
	// TODO: move this into menubar's draw handler
	drawBigNumber(game.w - 16*6, 32-2, match->deck_len / 10);
	drawBigNumber(game.w - 16*5, 32-2, match->deck_len % 10);
}
