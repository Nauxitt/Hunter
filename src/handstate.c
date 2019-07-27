#include <SDL2/SDL.h>
#include <stdlib.h>

#include "handstate.h"
#include "stateengine.h"
#include "hunter.h"
#include "sprites.h"
#include "draw.h"


HandState * makeHandState(HandState * state, Hunter * hunter, int x, int y){
	if(state == NULL)
		state = (HandState*) calloc(sizeof(HandState), 1);

	state->hunter = hunter;
	state->x = x;
	state->y = y;

	EventHandler(state)->type = "HandState";
	EventHandler(state)->onTick = handOnTick;
	EventHandler(state)->onDraw = handOnDraw;
	EventHandler(state)->onKeyUp = handOnKeyUp;
	EventHandler(state)->onMouseUp = handOnMouseUp;
	
	return state;
}

void handOnTick(EventHandler * h){}
void handOnMouseUp(EventHandler * h, SDL_Event * e){}

void handOnKeyUp(EventHandler * h, SDL_Event * e){
	HandState * state = HandState(h);

	// Calculate hand size
	int hand_size = hunterHandSize(state->hunter);
	Card * card = state->hunter->hand[state->selector];
	
	switch(e->key.keysym.scancode){
		case SDL_SCANCODE_LEFT:
			state->selector--;
			if(state->selector == -1)
				state->selector = hand_size - 1;
			break;

		case SDL_SCANCODE_RIGHT:
			state->selector++;
			if(state->selector >= hand_size)
				state->selector = 0;
			break;

		case SDL_SCANCODE_SPACE:
		case SDL_SCANCODE_RETURN:
			// TODO: handle NO_CARD
			*state->card_target = card;
			
			gamePopState();
			free(state);
			break;

		case SDL_SCANCODE_ESCAPE:
			*state->card_target = NULL;

			gamePopState();
			free(state);
			break;

		default:
			// Forward input to previous state
			onKeyUp(EventHandler(GameState(h)->prevState), e);
			break;
	}
	
}

void handOnDraw(EventHandler * h){
	// HandState is drawn on top of another state
	onDraw(EventHandler(GameState(h)->prevState));

	// TODO: draw a selector triangle instead of pulling the card image down.

	HandState * state = HandState(h);
	Hunter * hunter = state->hunter;
	
	SDL_Rect window_panel = {
			state->x,
			state->y,
			16 + textures.cards.w * 7,
			64
		};
	
	// Before ever drawing cards, animate the panel frame expanding from nothing to full size
	int expand_time = 175;
	int duration = GameState(state)->duration;
	float scale = (float) duration / (float) expand_time;
	if(scale < 1.0){
		// Horizontally center scaling
		window_panel.x += window_panel.w/2 - (float) window_panel.w * scale / 2;

		window_panel.w *= scale;
		window_panel.h *= scale;

		if(window_panel.w >= textures.statbox.w * 2)
			drawWindowPanel(state->color, &window_panel);

		return;
	}

	// Panel is done expanding, draw panel at full size.
	drawWindowPanel(state->color, &window_panel);

	// Draw cards
	for(int c=0; c < HAND_LIMIT; c++){
		Card * card = hunter->hand[c];
		
		if(card == NULL)
			break;

		SDL_Rect dest = {
				window_panel.x + 8 + c*textures.cards.w,
				window_panel.y + 16,
				textures.cards.w, textures.cards.h
			};

		if(state->selector == c)
			dest.y += 8;
		
		drawCard(dest.x, dest.y, card);
	}
}
