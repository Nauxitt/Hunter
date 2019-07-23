#include "stateengine.h"
#include <stdlib.h>
#include <time.h>

int initGame(){
	// Initialize random numbers
	srand(time(NULL));
	
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		return 1;

	// TODO: allow different window sizes
	game.w = 640;
	game.h = 480;

	game.window = SDL_CreateWindow(
			"Hunter of Battles",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			game.w, game.h,
			SDL_WINDOW_SHOWN
		);

	if (game.window == NULL){
		return 1;
		SDL_Quit();
	}

	game.renderer = SDL_CreateRenderer(game.window, -1, SDL_RENDERER_ACCELERATED);

	if(game.renderer == NULL){
		SDL_DestroyWindow(game.window);
		return 1;
	}

	return 0;
}

GameState * makeGameState(){
	return (GameState *) calloc(sizeof(GameState), 1);
}

EventHandler * makeEventHandler(){
	return (EventHandler *) calloc(sizeof(EventHandler), 1);
}

GameState * gamePushState(GameState * state){
	EventHandler * prev = (EventHandler *) game.state;
	if(prev && prev->onExit)
		prev->onExit(prev);

	state->prevState = game.state;
	game.state = state;

	if(state->events.onEnter)
		state->events.onEnter((EventHandler*) state);
	
	state->enterTime = SDL_GetTicks();
	return state;
}

GameState * gamePopState(){
	GameState * ret = game.state;
	if(ret->events.onExit)
		ret->events.onExit((EventHandler*) ret);
	
	game.state = ret->prevState;

	if(game.state->events.onEnter)
		game.state->events.onEnter((EventHandler*) game.state);

	return ret;
}

void gameProcessEvent(SDL_Event * e){
	EventHandler * handler = (EventHandler *) game.state;
	switch(e->type){
		case SDL_KEYUP:
			if(handler->onKeyUp)
				handler->onKeyUp(handler, e);
			break;

		case SDL_KEYDOWN:
			if(e->key.repeat){
				if(handler->onKeyHold)
					handler->onKeyHold(handler, e);
			}
			else {
				if(handler->onKeyDown)
					handler->onKeyDown(handler, e);
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			if(handler->onMouseDown)
				handler->onMouseDown(handler, e);
			break;

		case SDL_MOUSEBUTTONUP:
			if(handler->onMouseUp)
				handler->onMouseUp(handler, e);
			break;
	}
}

void blit(SDL_Texture * texture, SDL_Rect * src, SDL_Rect * dest){
	SDL_RenderCopy(game.renderer, texture, src, dest);
}

void gameCycle(){
	SDL_Event event;

	while(SDL_PollEvent(&event)){
		if(event.type == SDL_QUIT){
			game.quit = 1;
			return;
		}
		else
			gameProcessEvent(&event);
	}

	uint32_t time = SDL_GetTicks();
	game.state->duration = time - game.state->enterTime;
	
	if(game.state->events.onTick)
		game.state->events.onTick((EventHandler*) game.state);

	SDL_SetRenderDrawColor(game.renderer,0,0,0,255);
	SDL_RenderClear(game.renderer);

	if(game.state->events.onDraw)
		game.state->events.onDraw((EventHandler *) game.state);

	SDL_RenderPresent(game.renderer);
	SDL_Delay(16);
}

void gameMainLoop(){
	while(game.state && !game.quit)
		gameCycle();
	
	SDL_DestroyWindow(game.window);
	SDL_Quit();
}

void onTick(EventHandler * h){ if(h->onTick) h->onTick(h); }
void onDraw(EventHandler * h){ if(h->onDraw) h->onDraw(h); }

ActionQueue * makeAction(char * type){
	ActionQueue * ret = (ActionQueue*) malloc(sizeof(ActionQueue));
	ret->type = type;
	return ret;
}


ActionQueue * pushAction(char * type){
	ActionQueue * new_action = makeAction(type);
	new_action->next = game.action;
	new_action->start = SDL_GetTicks();
	game.action = new_action;
	return new_action;
}


int pollAction(char * type){
	if(game.action == NULL)
		return 0;

	return strcmp(game.action->type, type) == 0;
}


void nextAction(){
	ActionQueue * pop = game.action;
	game.action = game.action->next;
	free(pop);
}

void prevStateOnDraw(EventHandler * h){
	EventHandler * prev = EventHandler(GameState(h)->prevState);

	if(prev == NULL)
		return;

	if(prev->onDraw)
		prev->onDraw(prev);
}
