#include "statAllocatorPanel.h"
#include "hunter.h"
#include "draw.h"
#include "sprites.h"
#include "stateengine.h"

StatAllocatorState *  makeStatAllocatorState(StatAllocatorState * state, Hunter * hunter, int points){
	if(state == NULL)
		state = (StatAllocatorState*) calloc(sizeof(StatAllocatorState), 1);
	
	EventHandler(state)->type = "StatAllocatorState";
	EventHandler(state)->onDraw = statAllocatorStateOnDraw;
	EventHandler(state)->onKeyUp = statAllocatorStateOnKeyUp;

	state->selector = 1;
	state->rect.x = 16;
	state->rect.y = 64 + 16;
	state->rect.w = STAT_ALLOCATOR_W;
	state->rect.h = STAT_ALLOCATOR_H;
	state->hunter = hunter;
	state->points = points;

	memcpy(&state->stats, &hunter->base_stats, sizeof(Statset));
	return state;
}

void statAllocatorStateEnd(StatAllocatorState * state){
	// Don't exit if there's still points left to distribute
	if(state->points > 0)
		return;

	// Save stat data to hunter and exit
	memcpy(&state->hunter->base_stats, &state->stats, sizeof(Statset));
	gamePopState();
}

void statAllocatorStateOnKeyUp(EventHandler * h, SDL_Event * e){
	StatAllocatorState * state = StatAllocatorState(h);

	// Based of the selector and the Hunter's stats, get a pointer to the current stat, and the lowest allowed value of that stat
	uint8_t * stat = NULL;
	uint8_t stat_min = 0;
	
	switch(state->selector){
		case 0:
			stat = &state->stats.mov;
			stat_min = state->hunter->base_stats.mov;
			break;
		case 1:
			stat = &state->stats.atk;
			stat_min = state->hunter->base_stats.atk;
			break;
		case 2:
			stat = &state->stats.def;
			stat_min = state->hunter->base_stats.def;
			break;
		case 3:
			stat = &state->stats.max_hp;
			stat_min = state->hunter->base_stats.max_hp;
			break;
	}

	switch(e->key.keysym.scancode){
		// Move selector up, within bounds
		case SDL_SCANCODE_UP:
			if(--state->selector < 0)
				state->selector = 0;
			break;

		// Move selector down, within bounds
		case SDL_SCANCODE_DOWN:
			if(++state->selector > 4)
				state->selector = 4;
			break;

		// Stat deincrement
		case SDL_SCANCODE_LEFT:
			if(stat == NULL)
				break;

			if(*stat <= stat_min)
				break;

			state->points++;
			(*stat)--;
			break;

		// Stat increment
		case SDL_SCANCODE_RIGHT:
			if(stat == NULL)
				break;

			if(state->points <= 0)
				break;
			
			(*stat)++;
			state->points--;
			break;

		case SDL_SCANCODE_SPACE:
			if(state->selector == 4)
				statAllocatorStateEnd(state);
			break;

		case SDL_SCANCODE_RETURN:
			if(state->selector == 4)
				statAllocatorStateEnd(state);
			else
				state->selector = 4;

			break;

		default:
			break;
	}
}

void statAllocatorStateOnDraw(EventHandler * h){
	StatAllocatorState * state = StatAllocatorState(h);
	prevStateOnDraw(GameState(h));

	// Animate panel expansion, exit if not finished
	float expand_time = 175;
	float scale = (float) GameState(state)->duration / expand_time;
	if(scale < 1.0){
		drawWindowPanelScaled(state->color, &state->rect, scale);
		return;
	}
	drawWindowPanel(state->color, &state->rect);

	Statset * original_stats = &state->hunter->base_stats;
	int margin = 16;

	drawString(
			(char*) state->hunter->name,
			state->rect.x + margin,
			state->rect.y + margin
		);
	
	SDL_Rect src = {
		0, textures.statbox.src_h,
		textures.statbox.src_w * 2,
		textures.statbox.src_h,
	};

	SDL_Rect dest = {
		state->rect.x + margin + textures.font.w,
		state->rect.y + margin + textures.font.h,
		textures.statbox.w * 2,
		textures.statbox.h
	};

	// Selector position
	SDL_Rect selector = {
		dest.x + textures.font.w/2 - textures.font.w - (textures.font.w-8)/2,
		dest.y + state->selector * textures.statbox.h + 4,
		textures.font.w - 8,
		textures.font.w - 8,
	};
	
	// Bottom row is is vertically padded by one row of space
	if(state->selector >= 4)
		selector.y += textures.statbox.h;

	// Blit selector
	SDL_RenderFillRect(game.renderer, &selector);

	// Text column screen positions
	int point_count_x = dest.x + dest.w * 3/2;
	int modifier_x = dest.x + dest.w * 3;

	// Draw Mv.
	blit(textures.statbox.texture, &src, &dest);
	if(state->stats.mov >= 10)
		drawBigNumber(point_count_x, dest.y, state->stats.mov / 10);
	drawBigNumber(point_count_x+textures.font.w, dest.y, state->stats.mov % 10);

	drawBigNumber(point_count_x + textures.font.w * 6, dest.y, state->stats.mov/3);
	
	// Draw Atk: Position
	src.x += textures.statbox.src_w * 2;
	dest.y += textures.statbox.h;

	// Draw Atk: Label
	blit(textures.statbox.texture, &src, &dest);

	// Draw Atk: Points
	if(state->stats.atk >= 10)
		drawBigNumber(point_count_x, dest.y, state->stats.atk / 10);
	drawBigNumber(point_count_x+textures.font.w, dest.y, state->stats.atk % 10);

	// Draw Atk: Modifier
	int atk = state->stats.atk;
	if(atk >= 10)
		drawBigNumber(point_count_x + textures.font.w * 5, dest.y, atk / 10);
	drawBigNumber(point_count_x + textures.font.w * 6, dest.y, atk % 10);

	// Draw Def: Position
	src.x += textures.statbox.src_w * 2;
	dest.y += textures.statbox.h;

	// Draw Def: Label
	blit(textures.statbox.texture, &src, &dest);

	// Draw Def: Points
	if(state->stats.def >= 10)
		drawBigNumber(point_count_x, dest.y, state->stats.def / 10);
	drawBigNumber(point_count_x+textures.font.w, dest.y, state->stats.def % 10);

	// Draw Def: Modifier
	int def = state->stats.def / 2;
	if(def >= 10)
		drawBigNumber(point_count_x + textures.font.w * 5, dest.y, def/10);
	drawBigNumber(point_count_x + textures.font.w * 6, dest.y, def%10);

	// Draw Hp: Position
	src.x += textures.statbox.src_w * 2;
	dest.y += textures.statbox.h;

	// Draw Hp: Label
	blit(textures.statbox.texture, &src, &dest);

	// Draw Hp: Points
	if(state->stats.max_hp >= 10)
		drawBigNumber(point_count_x, dest.y, state->stats.max_hp / 10);
	drawBigNumber(point_count_x+textures.font.w, dest.y, state->stats.max_hp % 10);

	// Draw Hp: Modifier
	int hp = state->stats.max_hp * 3 + 6 + state->hunter->level;

	drawBigNumber(point_count_x + textures.font.w * 5, dest.y, hp/10);
	drawBigNumber(point_count_x + textures.font.w * 6, dest.y, hp%10);

	// Bottom row (END, points) position
	dest.y += textures.statbox.h * 2;

	// END button
	drawString("END", dest.x, dest.y);

	// Points remaining
	drawBigNumber(point_count_x + textures.font.w * 4, dest.y, state->points / 10);
	drawBigNumber(point_count_x + textures.font.w * 5, dest.y, state->points % 10);
}
