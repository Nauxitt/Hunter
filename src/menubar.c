#include "menubar.h"
#include "stateengine.h"
#include "hunter.h"
#include "sprites.h"
#include "draw.h"

MenubarState * initMenu(MenubarState * state){
	if(state == NULL)
		state = MenubarState(calloc(sizeof(MenubarState), 1));

	state->drawContents = drawMenubarContents;
	state->scroll_speed = 6;

	EventHandler(state)->type = "MenubarState";
	EventHandler(state)->onDraw = menuOnDraw;
	EventHandler(state)->onKeyUp = menuOnKeyUp;

	return state;
}

void menuOnKeyUp(EventHandler * h, SDL_Event * e){
	MenubarState * state = MenubarState(h);
	uint32_t time = GameState(h)->duration;

	switch(e->key.keysym.scancode){
		case SDL_SCANCODE_LEFT:
			if (--state->selector < 0) {
				state->selector = state->length-1;
				state->selector_change_time = time;
			}
			break;

		case SDL_SCANCODE_RIGHT:
			if (++state->selector >= state->length) {
				state->selector = 0;
				state->selector_change_time = time;
			}
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
	spritesheetBlit(
			&textures.menu_icons,
			id % textures.menu_icons.tiles_h,
			id / textures.menu_icons.tiles_h,
			x,y
		);
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

		drawMenubarIcon(i*38 + 32, 24, id);
	}
	
	// Draw scroling text window

	SDL_Rect src = {0, 80, 144, 16};

	SDL_Rect dest = {5*38 + 32, 24, 144*2, 32};
	blit(textures.menu_icons.texture, &src, &dest);

	// Skip drawing text if inactive
	if (menu->active == 0)
		return;

	// Skip drawing text if no icons are selected
	if (menu->selector == -1)
		return;

	// Skip drawing text if there's no text to draw
	char * help_text = menu->icons[menu->selector].help_text;
	if (help_text == NULL)
		return;

	// Set clip inside rect and draw text inside the textbox
	dest.x += 4;
	dest.y += 4;
	dest.w -= 8;
	dest.h -= 8;
	SDL_RenderSetClipRect(game.renderer, &dest);
	
	int scroll_length = dest.w + textures.font.w * strlen(help_text);
	int scroll_offset = (GameState(menu)->duration - menu->selector_change_time) / menu->scroll_speed % scroll_length;

	drawString(
			help_text,
			dest.x + dest.w - scroll_offset,
			dest.y
		);
	
	SDL_RenderSetClipRect(game.renderer, NULL);
}

void menuOnDraw(EventHandler * h){
	MenubarState * menu = MenubarState(h);
	stateUpdateTime((GameState*) menu, 0);

	if(menu->selector_change_time == 0)
		menu->selector_change_time = GameState(h)->duration;

	// Render menubar background
	SDL_Rect dest = {0, 0, game.w, 64};
	drawMenubarBackground(&dest);

	// Executed before feather drawing, and has the chance to modify activation status of menubar.
	if(menu->drawContents)
		menu->drawContents(menu);

	// Draw selector feather
	if(menu->active){
		SDL_Rect src = {0, 96, 32, 32};
		
		if(menu->selector != -1){
			dest.x = menu->selector * 38 + 32;
			dest.y = 24;
			dest.w = 64; dest.h = 64;
			blit(textures.menu_icons.texture, &src, &dest);
		}
	}
}
