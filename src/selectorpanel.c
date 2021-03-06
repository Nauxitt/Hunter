#include <SDL2/SDL.h>
#include <stdlib.h>

#include "selectorpanel.h"
#include "stateengine.h"
#include "hunter.h"
#include "sprites.h"
#include "draw.h"
#include "cards.h"

SelectorPanelState * makeSelectorPanelState(SelectorPanelState * state){
	if(state == NULL)
		state = (SelectorPanelState*) calloc(sizeof(SelectorPanelState), 1);

	// Some default sizes
	if(state->item_gutter == 0)
		state->item_gutter = 0;

	if(state->panel_margin == 0)
		state->panel_margin = 16;

	if((state->rect.w == 0) || (state->rect.h == 0)){
		state->rect.w = state->panel_margin*2 + state->icon.w*state->max_length;
		state->rect.h = state->icon.h + state->panel_margin*2;

		// make some room for the select none button
		if(state->select_none){
			state->rect.w += textures.cards.w;

			// If the panel is too small for the select none button, make the panel taller.
			int h = state->panel_margin*2 + textures.cards.h;
			if(state->rect.h < h)
				state->rect.h = h;
		}
	}
	
	EventHandler(state)->type = "SelectorPanelState";
	EventHandler(state)->onKeyUp = selectorPanelOnKeyUp;
	// EventHandler(state)->onMouseUp = selectorPanelOnMouseUp;
	EventHandler(state)->onDraw = selectorPanelOnDraw;

	return state;
}

SelectorPanelState * makeCardSelectState(SelectorPanelState * state, Hunter * hunter, int x, int y){
	if(state == NULL)
		state = (SelectorPanelState*) calloc(sizeof(SelectorPanelState), 1);

	state->color = (enum WindowColor) hunter->id;
	state->length = hunterHandSize(hunter);
	state->source = hunter->hand;
	state->rect.x = x;
	state->rect.y = y;
	state->icon.w = textures.cards.w;
	state->icon.h = textures.cards.h;
	state->select_none = 1;

	makeSelectorPanelState(state);
	EventHandler(state)->type = "CardSelectState";
	state->drawIcon = cardSelectDrawIcon;
	state->onChoose = cardSelectOnChoose;
	
	return state;
}

SelectorPanelState * makeInventorySelectState(SelectorPanelState * state, Hunter * hunter, int x, int y){
	if(state == NULL)
		state = (SelectorPanelState*) calloc(sizeof(SelectorPanelState), 1);

	state->color = (enum WindowColor) hunter->id;
	state->length = hunterInventoryLength(hunter);
	state->source = hunter->inventory;
	state->rect.x = x;
	state->rect.y = y;
	state->icon.w = textures.items.w;
	state->icon.h = textures.items.h;
	state->select_none = 1;

	makeSelectorPanelState(state);
	EventHandler(state)->type = "CardSelectState";
	state->drawIcon = relicSelectDrawIcon;
	state->onChoose = relicSelectOnChoose;
	
	return state;
}

void selectorPanelOnKeyUp(EventHandler * h, SDL_Event * e){
	SelectorPanelState * state = SelectorPanelState(h);

	int max = state->length;
	if(state->select_none)
		max++;
	
	switch(e->key.keysym.scancode){
		case SDL_SCANCODE_LEFT:
			state->selector--;
			if(state->selector == -1)
				state->selector = max - 1;
			break;

		case SDL_SCANCODE_RIGHT:
			state->selector++;
			if(state->selector >= max)
				state->selector = 0;
			break;

		case SDL_SCANCODE_SPACE:
		case SDL_SCANCODE_RETURN:
			// TODO: handle NO_CARD
			if(state->selector == state->length)
				state->onChoose(state, -1);
			else
				state->onChoose(state, state->selector);
			
			gamePopState();
			free(state);
			break;

		case SDL_SCANCODE_ESCAPE:
			if(state->cancellable){
				state->onCancel(state);
				gamePopState();
				free(state);
			}
			break;

		default:
			// Forward input to previous state
			onKeyUp(EventHandler(GameState(h)->prevState), e);
			break;
	}
}

void selectorPanelOnDraw(EventHandler *h){
	// SelectorPanelState is drawn on top of another state
	onDraw(EventHandler(GameState(h)->prevState));

	// TODO: draw a selector triangle instead of pulling the icon image down.

	SelectorPanelState * state = SelectorPanelState(h);

	SDL_Rect window_panel = {
			state->rect.x, state->rect.y,
			state->panel_margin*2 + (state->icon.w+state->item_gutter) * state->length - state->item_gutter,
			state->panel_margin*2 + state->icon.h
		};

	if(state->select_none)
		window_panel.w += state->icon.w + state->item_gutter;
	
	// Before ever drawing cards, animate the panel frame expanding from nothing to full size
	int expand_time = 175;
	int duration = GameState(state)->duration;
	float scale = (float) duration / (float) expand_time;
	if(scale < 1.0){
		drawWindowPanelScaled(state->color, &window_panel, scale);
		return;
	}

	// Panel is done expanding, draw panel at full size.
	drawWindowPanel(state->color, &window_panel);

	// Draw icons
	for(int n=0; n < state->length; n++){
		int x = state->rect.x + state->panel_margin + n * (state->icon.w + state->item_gutter);
		int y = state->rect.y + state->panel_margin;

		if(state->selector == n)
			y += 8;

		state->drawIcon(state, n, x, y);
	}

	// If this mode is on, draw no item selection button.
	if(state->select_none){
		// TODO: blit cancel button from a spritesheet
		SDL_Rect dest = {
			state->rect.x + state->panel_margin + state->length * (state->icon.w+state->item_gutter),
			state->rect.y + state->panel_margin,
			state->icon.w,
			state->icon.h
		};

		if(state->selector == state->length)
			dest.y += 8;

		SDL_SetRenderDrawColor(game.renderer, 255,0,0,255);
		SDL_RenderFillRect(game.renderer, &dest);
	}
}

void cardSelectDrawIcon(SelectorPanelState * state, int n, int x, int y){
	drawCard(x, y, ((Card**)state->source)[n]);
}

void cardSelectOnChoose(SelectorPanelState * state, int n){
	if(n == -1)
		*state->target = &no_card;
	else
		*state->target = ((Card**) state->source)[n];
}

void relicSelectDrawIcon(SelectorPanelState * state, int n, int x, int y){
	drawRelic(((Relic**)state->source)[n], x, y);
}

void relicSelectOnChoose(SelectorPanelState * state, int n){
	if(n == -1)
		*state->target = NULL;
	else
		*state->target = ((Relic**)state->source)[n];
}
