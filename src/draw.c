#include <SDL2/SDL.h>
#include "draw.h"
#include "stateengine.h"
#include "hunter.h"
#include "sprites.h"
#include "utils.h"

void drawWindowPanel(enum WindowColor color, SDL_Rect * window_dest){
	SDL_SetRenderDrawBlendMode(game.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 32);
	// SDL_SetRenderDrawColor(game.renderer, 255-32, 255, 255, 32);
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

void drawSmallNumber(int x, int y, int n){
	spritesheetBlit(&textures.small_numbers, n,0, x,y);
}

void drawBigNumber(int x, int y, int n){
	spritesheetBlit(&textures.statbox, n,0, x,y);
}
		
void drawBigRedNumber(int x, int y, int n){
	SDL_SetTextureColorMod(textures.statbox.texture, 255, 0, 0);
	drawBigNumber(x, y, n);
	SDL_SetTextureColorMod(textures.statbox.texture, 255, 255, 255);
}

void drawCard(int x, int y, Card * card){
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

void drawRelic(Relic * relic, int x, int y){
	spritesheetBlit(&textures.items, relic->item_id,0, x, y);
}

void drawStatboxStats(Hunter * hunter, int x, int y){
	Statset * stats = hunterStats(hunter);

	int panel_w = (game.w - 16*2 - 4*3) / 4;
	SDL_Rect panel_rect = {x, y, panel_w, 160};

	int element_gutter = 4;
	int element_margin = 18;

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
	spritesheetBlit(
			&textures.statbox,
			3 + ((hunter->stats.hp == 0) ? 3:0), 2,
			statval_dest.x, statval_dest.y
		);
	
	// Healthbar middle
	int middle_w = panel_rect.w - element_margin - textures.statbox.w*2;

	SDL_Rect statbar_src;
	statval_dest.x += textures.statbox.w;
	statval_dest.w = middle_w;
	getSpriteClip(&textures.statbox, 7, 2, &statbar_src);
	blit(textures.statbox.texture, &statbar_src, &statval_dest);
	
	// The first and last HP points are drawn by the left and right edge blits respectively, so draw the middle portion of the healthbar in proportion of health to max health, precluding representation of those values.
	if(hunter->base_stats.hp <= 1)
		statval_dest.w = 0;
	else if(hunter->base_stats.hp >= hunter->base_stats.max_hp - 1)
		statval_dest.w = middle_w;
	else
		statval_dest.w = middle_w * hunter->base_stats.hp / (hunter->base_stats.max_hp-1);

	getSpriteClip(&textures.statbox, 4, 2, &statbar_src);
	blit(textures.statbox.texture, &statbar_src, &statval_dest);

	// Draw HP number: max HP 1's
	statval_dest.y -= textures.small_numbers.h - 4;
	statval_dest.x = panel_rect.x + panel_rect.w - element_margin/2 - textures.small_numbers.w;

	drawSmallNumber(
			XY(&statval_dest), hunter->base_stats.max_hp % 10
		);

	
	// Draw HP number: max HP 10's
	statval_dest.x -= textures.small_numbers.w;
	drawSmallNumber(
			XY(&statval_dest), hunter->base_stats.max_hp / 10
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
			XY(&statval_dest), hunter->base_stats.hp % 10
		);

	// Draw HP number: current HP 10's
	statval_dest.x -= textures.small_numbers.w;
	drawSmallNumber(
			XY(&statval_dest), hunter->base_stats.hp / 10
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
