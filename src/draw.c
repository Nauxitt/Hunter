#include <SDL2/SDL.h>
#include "draw.h"
#include "stateengine.h"
#include "hunter.h"
#include "sprites.h"
#include "utils.h"

void drawWindowPanel(enum WindowColor color, SDL_Rect * window_dest){
	SDL_SetRenderDrawBlendMode(game.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 32);

	// active player background color ^
	SDL_RenderFillRect(game.renderer, window_dest);
	SDL_SetRenderDrawBlendMode(game.renderer, SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(game.renderer, 255, 255, 255, 255);
	
	// Panel inset border
	SDL_Rect border = *window_dest;
	border.x += 2; border.y += 2;
	border.w -= 4; border.h -= 4;
	SDL_RenderDrawRect(game.renderer, &border);

	//Draw bar middle
	SDL_Rect src;
	getSpriteClip(&textures.statbox, 1,2+color, &src);
	SDL_Rect dest = {window_dest->x, window_dest->y, window_dest->w, textures.statbox.h};
	blit(textures.statbox.texture, &src, &dest);
	
	// Draw bar left
	getSpriteClip(&textures.statbox, 0,2+color, &src);
	dest.w = textures.statbox.w;
	blit(textures.statbox.texture, &src, &dest);

	// Draw bar right
	getSpriteClip(&textures.statbox, 2,2+color, &src);
	dest.x += window_dest->w - textures.statbox.w;
	blit(textures.statbox.texture, &src, &dest);
}

void drawWindowPanelScaled(enum WindowColor color, SDL_Rect * window_dest, float scale){
	if(scale >= 1){
		drawWindowPanel(color, window_dest);
		return;
	}

	SDL_Rect rect = {
		window_dest->x,
		window_dest->y,
		window_dest->w,
		window_dest->h
	};

	// Horizontally center scaling
	rect.x += rect.w/2 - (float) rect.w * scale / 2;

	rect.w *= scale;
	rect.h *= scale;

	if(rect.w >= textures.statbox.w * 2)
		drawWindowPanel(color, &rect);
}

void drawSmallNumber(int x, int y, int n) {
	spritesheetBlit(&textures.small_numbers, n,0, x,y);
}

void drawBigNumber(int x, int y, int n) {
	spritesheetBlit(&textures.statbox, n,0, x,y);
}
		
void drawBigRedNumber(int x, int y, int n) {
	SDL_SetTextureColorMod(textures.statbox.texture, 255, 0, 0);
	drawBigNumber(x, y, n);
	SDL_SetTextureColorMod(textures.statbox.texture, 255, 255, 255);
}

void drawCard(int x, int y, Card * card) {
	int sx, sy;

	switch(card->type){
		case MOVE_CARD:         
		case MOVE_EXIT_CARD:    
			sx = card->num;
			sy = 0;
			break;

		case DEFENSE_ALL_CARD:  
		case DEFENSE_DOUBLE_CARD:
		case DEFENSE_CARD:      
			sx = card->num;
			sy = 1;
			break;

		case ATTACK_DOUBLE_CARD:
		case ATTACK_COPY_CARD:  
		case ATTACK_CARD:       
			sx = card->num;
			sy = 2;
			break;

		case EMPTY_TRAP_CARD:   
		case STUN_TRAP_CARD:    
		case DAMADE_TRAP_CARD:  
		case LEG_DAMAGE_CARD:   
			sx = 0;
			sy = 3;
			break;

		case NO_CARD:
		case UNKNOWN_CARD:
			break;
	}

	spritesheetBlit(&textures.cards, sx,sy, x,y);
}

void drawStatbox(Hunter * hunter, enum StatboxViews view, enum WindowColor color, int x, int y){
	if(view == STATBOX_VIEW_NONE)
		return;

	// int panel_gutter = 4;
	int panel_w = (game.w - 16*2 - 4*3) / 4;
	
	// Draw panel
	SDL_Rect panel_rect = {x, y, panel_w, 160};
	drawWindowPanel(color, &panel_rect);
	
	if(view == STATBOX_VIEW_STATS)
		drawStatboxStats(hunter, x, y);
	else if(view == STATBOX_VIEW_ITEMS)
		drawStatboxItems(hunter, x, y);
}

void drawStatboxItems(Hunter * hunter, int x, int y){
	int panel_w = (game.w - 16*2 - 4*3) / 4;
	int element_margin = 18;
	int element_gutter = (panel_w - 2*element_margin - 3*textures.items.w) / 2;

	for(int r=0; r < INVENTORY_LIMIT; r++){
		Relic * relic = hunter->inventory[r];
		if(relic == NULL)
			break;
		
		drawRelic(
				relic, 
				x + element_margin + (textures.items.w+element_gutter) * (r % 3),
				y + 160 - element_margin + (textures.items.h + element_gutter) * (r/3 - 2)
			);
	}
}

void drawRelic(Relic * relic, int x, int y) {
	spritesheetBlit(&textures.items, relic->item_id,0, x, y);
}

void drawStatboxStats(Hunter * hunter, int x, int y) {
	Statset * stats = hunterStats(hunter);

	int panel_w = (game.w - 16*2 - 4*3) / 4;
	SDL_Rect panel_rect = {x, y, panel_w, 160};

	int element_gutter = 4;
	int element_margin = 18;

	drawString(
			(char*) &hunter->name,
			x+4 + panel_w/2-element_gutter - strlen((char*) &hunter->name)*textures.font.w/2,
			y + element_margin
		);

	// Draw stat names
	//    Mv. stat
	SDL_Rect statname_src = {
			0, textures.statbox.src_h,
			textures.statbox.src_w * 2,     // takes up two tiles
			textures.statbox.src_h
		};

	SDL_Rect statname_dest = {
			panel_rect.x + element_margin,
			panel_rect.y + (element_gutter + textures.statbox.h) * 2,
			textures.statbox.w * 2,
			textures.statbox.h
		};

	blit(textures.statbox.texture, &statname_src, &statname_dest);

	//    Mv. +
	SDL_Rect statval_src  = statname_src;
	statval_src.x = textures.statbox.src_w * 8;
	statval_src.w = textures.statbox.src_w;

	SDL_Rect statval_dest = statname_dest;
	statval_dest.y += textures.statbox.h + 2;
	statval_dest.w = textures.statbox.w;
	blit(textures.statbox.texture, &statval_src, &statval_dest);

	//    Mv. #
	statval_dest.x += textures.statbox.w;
	statval_src.y = 0;
	statval_src.x = stats->mov * textures.statbox.src_w;

	drawBigNumber(statval_dest.x, statval_dest.y, stats->mov);

	//    At. stat label
	statname_src.x += textures.statbox.src_w * 2;
	statname_dest.x += textures.statbox.w*2 + element_gutter;
	blit(textures.statbox.texture, &statname_src, &statname_dest);

	//    At. stat 10's digit
	statval_dest.x = statname_dest.x;
	if(stats->atk / 10){
		statval_src.x = (stats->atk/10) * textures.statbox.src_w;
		blit(textures.statbox.texture, &statval_src, &statval_dest);
	}
	//    At. stat 1's digit
	statval_dest.x += textures.statbox.w;
	drawBigNumber(statval_dest.x, statval_dest.y, stats->atk % 10);

	//    Df. stat label
	statname_dest.x += textures.statbox.w*2 + element_gutter;
	blit(textures.statbox.texture, &statname_src, &statname_dest);

	//    Df. stat 10's digit
	statval_dest.x = statname_dest.x;
	if(stats->def / 10){
		drawBigNumber(statval_dest.x, statval_dest.y, stats->def / 10);
	}
	//    Df. stat 1's digit
	statval_dest.x += textures.statbox.w;
	drawBigNumber(statval_dest.x, statval_dest.y, stats->def % 10);
	

	// Draw player hand
	int card_y = panel_rect.y + panel_rect.h - element_margin/2 - textures.cards.h;
	
	for(int c = hunterHandSize(hunter)-1; c >= 0; c--){
		Card * card = hunter->hand[c];
		int card_x = panel_rect.x + element_margin/2 + c * textures.cards.w/2;

		drawCard(card_x, card_y, card);
	}

	// Healthbar right edge
	statval_dest.y = card_y - textures.statbox.h - 2;
	statval_dest.x = panel_rect.x + panel_rect.w - element_margin/2 - textures.statbox.w;
	spritesheetBlit(
			&textures.statbox,
			5 + ((hunter->stats.hp < hunter->stats.max_hp) ? 3:0), 2,
			statval_dest.x, statval_dest.y
		);

	// Healthbar left edge
	statval_dest.x = panel_rect.x + element_margin/2;
	if (hunter->stats.hp == 0) {
		SDL_SetTextureAlphaMod(
				textures.statbox.texture,
				120
			);

		spritesheetBlit(
				&textures.statbox,
				3 + ((hunter->stats.restricted_hp < hunter->stats.max_hp) ? 3:0), 2,
				statval_dest.x, statval_dest.y
			);
	
		SDL_SetTextureAlphaMod(textures.statbox.texture, 255);
	}
	else {
		spritesheetBlit(
				&textures.statbox, 3, 2,
				statval_dest.x, statval_dest.y
			);
	}
	
	// Healthbar middle
	int middle_w = panel_rect.w - element_margin - textures.statbox.w*2;

	SDL_Rect statbar_src;
	statval_dest.x += textures.statbox.w;
	statval_dest.w = middle_w;
	getSpriteClip(&textures.statbox, 7, 2, &statbar_src);
	blit(textures.statbox.texture, &statbar_src, &statval_dest);

	// Draw restricted HP bar

	if (hunter->stats.restricted_hp < hunter->stats.max_hp) {
		statval_dest.w = middle_w * hunter->stats.restricted_hp / (hunter->stats.max_hp-1);
		SDL_SetTextureAlphaMod(
				textures.statbox.texture,
				120
			);
		getSpriteClip(&textures.statbox, 4, 2, &statbar_src);
		blit(textures.statbox.texture, &statbar_src, &statval_dest);
		SDL_SetTextureAlphaMod(textures.statbox.texture, 255);
	}
	
	// The first and last HP points are drawn by the left and right edge blits respectively, so draw the middle portion of the healthbar in proportion of health to max health, precluding representation of those values.
	if(hunter->stats.hp <= 1)
		statval_dest.w = 0;
	else if(hunter->stats.hp >= hunter->stats.max_hp - 1)
		statval_dest.w = middle_w;
	else
		statval_dest.w = middle_w * hunter->stats.hp / (hunter->stats.max_hp-1);

	getSpriteClip(&textures.statbox, 4, 2, &statbar_src);
	blit(textures.statbox.texture, &statbar_src, &statval_dest);

	// Draw HP number: max HP 1's
	statval_dest.y -= textures.small_numbers.h - 4;
	statval_dest.x = panel_rect.x + panel_rect.w - element_margin/2 - textures.small_numbers.w;

	drawSmallNumber(
			XY(&statval_dest), hunter->stats.max_hp % 10
		);

	
	// Draw HP number: max HP 10's
	statval_dest.x -= textures.small_numbers.w;
	drawSmallNumber(
			XY(&statval_dest), hunter->stats.max_hp / 10
		);

	// Draw HP number: slash
	statval_dest.x -= textures.statbox.w;
	spritesheetBlit(
			&textures.statbox, 9, 1,
			statval_dest.x, statval_dest.y - 6
		);

	// Draw HP number: current HP 1's
	statval_dest.x -= textures.small_numbers.w;
	drawSmallNumber(
			XY(&statval_dest), hunter->stats.hp % 10
		);

	// Draw HP number: current HP 10's
	statval_dest.x -= textures.small_numbers.w;
	drawSmallNumber(
			XY(&statval_dest), hunter->stats.hp / 10
		);

	// Draw HP number: HP label
	statval_dest.x = panel_rect.x + element_margin/2;
	statval_dest.y -= 6;
	statval_dest.w = textures.statbox.w * 2;
	statval_dest.h = textures.statbox.h;

	getSpriteClip(&textures.statbox, 6,1, &statval_src);
	statval_src.w *= 2;
	
	blit(textures.statbox.texture, &statval_src, &statval_dest);
}

void drawDiceBack(SDL_Rect * dest){
	SDL_Rect src;
	getSpriteClip(&textures.dice, 6,0, &src);
	blit(textures.dice.texture, &src, dest);
}

void drawDamageDice(int num, int x, int y){
	spritesheetBlit(&textures.dice, num-1,2, x,y);
}

void drawMoveDice(int num, int x, int y){
	spritesheetBlit(&textures.dice, num-1,0, x,y);
}

void drawDefenseDice(int num, int x, int y){
	spritesheetBlit(&textures.dice, num-1,1, x,y);
}

void drawDeckIndicator(int x, int y, int len){
	// Draw deck icon
	spritesheetBlit(&textures.menu_icons, 5,0, x, y);

	// Draw deck card count
	y += 6;
	drawBigNumber(x += textures.menu_icons.w, y, len / 10);
	drawBigNumber(x += textures.statbox.w   , y, len % 10);
}

void drawWallpaper(int id){
	spritesheetBlit(
			&textures.wallpapers,
			id % textures.wallpapers.tiles_h,
			id / textures.wallpapers.tiles_h,
			0, 68
		);
}

void drawWallpaperTransition(int id, int gap){
	int sx = id % textures.wallpapers.tiles_h;
	int sy = id / textures.wallpapers.tiles_h;
	SDL_Rect src;
	SDL_Rect dest = {-gap, 68, textures.wallpapers.w/2, textures.wallpapers.h};
	getSpriteClip(&textures.wallpapers, sx, sy, &src);
	src.w /= 2;

	// Draw left half
	blit(textures.wallpapers.texture, &src, &dest);

	// Draw right half
	dest.x = game.w/2 + gap;
	src.x += src.w;
	blit(textures.wallpapers.texture, &src, &dest);
}

void drawChar(char c, int x, int y){
	if('0' <= c && c <= '9')
		spritesheetBlit(&textures.font, c-'0',2, x,y);
	else if ('A' <= c && c <= 'Z')
		spritesheetBlit(&textures.font, c-'A',0, x,y);
	else if ('a' <= c && c <= 'z')
		spritesheetBlit(&textures.font, c-'a',1, x,y);
	else {
		int sx = 18;
		switch(c){
			case '&': sx =  0; break;
			case '%': sx =  1; break;
			case '?': sx =  2; break;
			case '!': sx =  3; break;
			case 1  : sx =  4; break;  // heart
			case '-': sx =  5; break;
			case 2  : sx =  6; break;  // list dot
			case '.': sx =  7; break;
			case ',': sx =  8; break;
			case 3  : sx =  9; break;  // left-quote
			case 4  : sx = 10; break;  // right-quote
			case '[': sx = 11; break;
			case ']': sx = 12; break;
			case '+': sx = 13; break;
			// case ''-': sx = 14; break;  TODO: minus, but not dash
			// case 'x': sx = 15; break; TODO: multiplication cross
			default:
				  break;
		}
		spritesheetBlit(&textures.font, sx,3, x,y);

	}

}

void drawString(char * str, int x, int y){
	while(*str){
		drawChar(*str, x,y);
		x += textures.font.w;
		str++;
	}
}
