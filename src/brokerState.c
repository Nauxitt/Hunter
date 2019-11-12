#include "brokerState.h"
#include "stateengine.h"
#include "sprites.h"
#include "draw.h"
#include "menubar.h"
#include "scorestate.h"
#include "mainMenuState.h"

BrokerState * makeBrokerState(BrokerState * state){
	if(state == NULL)
		state = (BrokerState *) calloc(sizeof(BrokerState), 1);

	EventHandler(state)->type = "BrokerState";
	EventHandler(state)->onPush  = brokerStateOnPush;
	EventHandler(state)->onPop   = brokerStateOnPop;
	EventHandler(state)->onDraw  = brokerStateOnDraw;
	EventHandler(state)->onKeyUp = brokerStateOnKeyUp;

	state->menubar = initMenu(NULL, NULL);
	state->menubar->drawContents = drawMenubarContents;
	state->menubar->selector = 0;
	state->menubar->active = 1;
	state->menubar->length = 4;
	state->menubar->icons[0].id =  9;
	state->menubar->icons[1].id = 10;
	state->menubar->icons[2].id = 11;
	state->menubar->icons[3].id = -1;

	return state;
}

void brokerStateOnDraw(EventHandler * h){
	BrokerState * state = BrokerState(h);
	drawWallpaper(22);
	spritesheetBlit(
			&textures.character_portraits,
			0,0,
			game.w-textures.character_portraits.w,
			game.h-textures.character_portraits.h
		);

	onDraw(EventHandler(state->menubar));
	onDraw(EventHandler(state->statbox));
}

void brokerStateOnKeyUp(EventHandler * h, SDL_Event * e){
	BrokerState * state = BrokerState(h);

	switch(e->key.keysym.scancode){
		case SDL_SCANCODE_RETURN:
		case SDL_SCANCODE_SPACE:
			gamePopState();
			mainMenuStartBasicMission(MainMenuState(GameState(state)->prevState));
			return;
		
		case SDL_SCANCODE_TAB:
			onKeyUp(EventHandler(state->statbox), e);
			break;

		case SDL_SCANCODE_LEFT:
		case SDL_SCANCODE_RIGHT:
			onKeyUp(state->menubar, e);
			break;

		case SDL_SCANCODE_ESCAPE:
			gamePopState();
			break;

		default:
			break;
	}
}

void brokerStateOnPush(EventHandler * h){
	SpeechBubbleState * bubble = makeSpeechBubbleState(NULL, "Hey, you!!\nWhat do you want?", 0, 0);
	bubble->rect.x = game.w-textures.character_portraits.w - 10 - bubble->rect.w;
	bubble->rect.y = game.h - (textures.character_portraits.h*2/3);
	gamePushState((GameState*) bubble);
}

void brokerStateOnPop(EventHandler * h){
	mainMenuTransitionIn(MainMenuState(game.state));
}

SpeechBubbleState * makeSpeechBubbleState(SpeechBubbleState * state, char * dialogue, int x, int y){
	if(state == NULL)
		state = (SpeechBubbleState *) calloc(sizeof(SpeechBubbleState), 1);

	EventHandler(state)->type = "SpeechBubbleState";
	EventHandler(state)->onDraw  = speechBubbleStateOnDraw;
	EventHandler(state)->onKeyUp = speechBubbleStateOnKeyUp;

	state->dialogue = dialogue;
	state->rect.x = x;
	state->rect.y = y;
	state->cw = 20;
	state->ch =  2;
	state->margin = textures.font.h/2;
	state->speed = 1000/40;

	speechBubbleUpdateSize(state);

	return state;
}

void speechBubbleUpdateSize(SpeechBubbleState * state){
	state->rect.w = state->margin*2 + state->cw * textures.font.w;
	state->rect.h = state->margin*2 + state->ch * textures.font.h;
}

void speechBubbleStateOnDraw(EventHandler * h){
	prevStateOnDraw(h);

	SpeechBubbleState * state = (SpeechBubbleState *) h;
	speechBubbleUpdateSize(state);

	state->rect.w = textures.font.w*state->cw + state->margin*2;
	SDL_SetRenderDrawColor(game.renderer, 0,0,0, 255);
	SDL_RenderFillRect(game.renderer, &state->rect);
	
	// Everything below is dialogue, so skip NULL dialogue
	if(state->dialogue == NULL)
		return;

	char buffer[512];
	int n = 0;
	int column = 0;
	int word = 0;
	char * c = state->dialogue;

	// Zero-terminate after linebreaks, and word-wrap along the way
	for(; *c != 0; c++, n++, column++){
		word++;
		buffer[n] = *c;
		
		if(*c == '\n'){
			buffer[++n] = 0;
			column = -1;
			word = 0;
		}
		else if(*c == ' '){
			word = 0;
			if(column >= state->cw-1){
				buffer[n] = '\n';
				buffer[++n] = 0;
				column = -1;
			}
		}
		else if(column >= state->cw-1){
			// For a really long word, break in the middle of the word.
			// otherwise, find the beginning of the word and split the line.
			if(word >= state->cw){
				buffer[n++] = '\n';
				buffer[n++] = 0;
				buffer[n++] = *c;
				column = 0;
			}
			else {
				// Shift characters forward by one
				for(int s=0; s < word; s++)
					buffer[n-s+1] = buffer[n-s];

				// Convert last space into linebreak, and terminate after that
				buffer[n-word]   = '\n';
				buffer[++n-word] = 0;

				column = word-1;
			}
		}
	}

	// Terminate the collection of strings with two zero-bytes
	buffer[n++] = 0;
	buffer[n]   = 0;
	
	// Draw character-by-character by truncating the string based on the state duration
	if(state->speed > 0 && !state->skip){
		int limit = GameState(state)->duration / state->speed;
		if(limit + 1 < n){
			buffer[limit] = 0;
			buffer[limit+1] = 0;
		}
		else
			state->skip = 1;
	}

	// Print each line
	char * line = (char*) &buffer;
	for(int y=0; *line; y++){
		drawString(
				line,
				state->rect.x + state->margin,
				state->rect.y + state->margin + textures.font.h*y
			);
		
		// Track within the buffer to the byte after the next terminator
		while(*line++);
	}
}

void speechBubbleStateOnKeyUp(EventHandler * h, SDL_Event * e){
	SpeechBubbleState * state = (SpeechBubbleState*) h;

	switch(e->key.keysym.scancode){
		case SDL_SCANCODE_RETURN:
		case SDL_SCANCODE_SPACE:
		case SDL_SCANCODE_ESCAPE:
			if(state->skip)
				gamePopState();
			else
				state->skip = 1;
			break;

		default:
			break;
	}
}
