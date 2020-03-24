#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "stateengine.h"
#include <stdlib.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <time.h>

int initGame(){
	// Initialize random numbers
	srand(time(NULL));

	if(TTF_Init() != 0)
		return 1;

	if((game.font = TTF_OpenFont("resources/Ubuntu-M.ttf", 24)) == NULL){
		printf("Could not load font.\n");
		return 1;
	}

	// if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
	if(SDL_Init(SDL_INIT_VIDEO) != 0)
		return 1;

	// TODO: allow different window sizes
	game.w = 640;
	game.h = 480;

	game.window = SDL_CreateWindow(
			"Hunter of Battles",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			game.w, game.h,
#ifdef _WIN32
			SDL_WINDOW_SHOWN
#else
			SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
#endif
		);

	if (game.window == NULL){
		SDL_Quit();
		return 1;
	}

	game.renderer = SDL_CreateRenderer(game.window, -1, SDL_RENDERER_ACCELERATED);

	if(game.renderer == NULL){
		SDL_DestroyWindow(game.window);
		SDL_Quit();
		return 1;
	}

#ifdef __EMSCRIPTEN__
	game.tick_delay = 0;
#else
	game.tick_delay = 16;
#endif

	return 0;
}

GameState * makeGameState(){
	GameState * ret = (GameState *) calloc(sizeof(GameState), 1);
	EventHandler(ret)->type = "GameState";
	return ret;
}

EventHandler * makeEventHandler(){
	EventHandler * ret = (EventHandler *) calloc(sizeof(EventHandler), 1);
	ret->type = "EventHandler";
	return ret;
}

GameState * gamePushState(GameState * state){
	EventHandler * prev = (EventHandler *) game.state;
	if(prev && prev->onExit)
		prev->onExit(prev);

	state->prevState = game.state;
	game.state = state;

	if(state->events.onPush)
		state->events.onPush((EventHandler*) state);

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

	if(ret && ret->events.onPop)
		ret->events.onPop(EventHandler(ret));
	
	if(game.state && game.state->events.onEnter)
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
	GameState * state = game.state;
	while(state != NULL){
		stateUpdateTime(state, time);
		state = state->prevState;
	}
	
	// Run gamestate tick handler, as well as those of any new GameStates which gets pushed onto the GameState stack from a prior tick handler.
	GameState * tickedState;
	do {
		if(game.state == NULL)
			return;

		tickedState = game.state;
		onTick(EventHandler(game.state));
	} while(game.state != tickedState);


	SDL_SetRenderDrawColor(
			game.renderer,
			game.background_color.r,
			game.background_color.g,
			game.background_color.b,
			255
		);
	SDL_RenderClear(game.renderer);

	if(game.state->events.onDraw)
		game.state->events.onDraw((EventHandler *) game.state);

	SDL_RenderPresent(game.renderer);

	if (game.tick_delay)
		SDL_Delay(game.tick_delay);
}

void gameMainLoop(){
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(gameCycle, 0, 1);
#else
	while(game.state && !game.quit)
		gameCycle();
	
	SDL_DestroyWindow(game.window);
	SDL_Quit();
#endif
}

uint32_t stateUpdateTime(GameState * state, uint32_t time){
	if (time == 0)
		time = SDL_GetTicks();
	
	if (state->enterTime == 0)
		state->enterTime = time;

	state->duration = time - state->enterTime;
	return time;
}

void onTick(EventHandler * h){ if(h->onTick) h->onTick(h); }
void onDraw(EventHandler * h){ if(h->onDraw) h->onDraw(h); }
void onKeyUp(EventHandler * h, SDL_Event * e){ if(h->onKeyUp) h->onKeyUp(h, e); }
void onKeyDown(EventHandler * h, SDL_Event * e){ if(h->onKeyUp) h->onKeyUp(h, e); }
void onMouseUp(EventHandler * h, SDL_Event * e){ if(h->onMouseUp) h->onMouseUp(h, e); }
void onMouseDown(EventHandler * h, SDL_Event * e){ if(h->onMouseUp) h->onMouseUp(h, e); }

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

void popEvent(EventHandler * h){
	gamePopState();
}

void freeEvent(EventHandler * h){
	free(h);
}

void allocationStateOnPop(EventHandler * h){
	AllocationState * state = (AllocationState *) h;

	// Recurse into internal allocation state stack
	if(state->prev_allocation)
		EventHandler(state->prev_allocation)->onPop(EventHandler(state->prev_allocation));

	free(state);
}

void * gameCalloc(int size, int n){
	int length = sizeof(AllocationState)+size*n;
	AllocationState * state = (AllocationState*) malloc(length);
	memset(state, 0, length);
	
	state->length = size * n;

	EventHandler(state)->type = "AllocationState";
	EventHandler(state)->onTick = popEvent;
	EventHandler(state)->onPop = allocationStateOnPop;
	
	EventHandler * prev = EventHandler(game.state);
	if(prev && prev->type && (strcmp(prev->type, "AllocationState") == 0)){
		state->prev_allocation = ((AllocationState*) prev);
		GameState(state)->prevState = game.state->prevState;
		game.state = (GameState*) state;
	}
	else {
		gamePushState((GameState*) state);
	}

	return state + 1;
}

GameState * gamePushStateOnTick(GameState * state){
	StateContainerState * container = (StateContainerState*) calloc(sizeof(StateContainerState), 1);
	EventHandler(container)->type = "ReplaceOnTickState";
	EventHandler(container)->onTick = containerStateReplace;
	EventHandler(container)->onPop = freeEvent;
	container->value = state;
	return gamePushState((GameState*) container);
}

void containerStateReplace(EventHandler * h){
	GameState * new = ((StateContainerState*) h)->value;
	gamePopState();
	gamePushState(new);
}
