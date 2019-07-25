#include "draw.h"
#include "stateengine.h"
#include "hunter.h"
#include "sprites.h"

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

void drawBigNumber(int x, int y, int n){
	spritesheetBlit(&textures.statbox, n,0, x,y);
}

void drawCard(int x, int y, Card * card){
	int sx, sy;

	switch(card->type){
		case NULL_CARD:

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

	int element_gutter = 8;
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
}