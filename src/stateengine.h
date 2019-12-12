/*
	Provides structures for defining states of different event handlers to SDL events, and provides a basic game loop which passes events to these states.  Three important structures are defined here: Game, EventHandler, and GameState.
*/

#ifndef __stateengine_h
#define __stateengine_h

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct _Game Game;
typedef struct _EventHandler EventHandler;
typedef struct _GameState GameState;
typedef struct _AllocationState AllocationState;
typedef struct _ActionQueue ActionQueue;

extern Game game;

/*
   Central container for the game's current state, and data for SDL rendering
*/
struct _Game {
	GameState * state;
	ActionQueue * action;
	uint8_t quit;
	int w, h;
	
	SDL_Window   * window;
	SDL_Renderer * renderer;
	TTF_Font * font;

	SDL_Color background_color;
};


/*
	Container of function pointers to various events.  Can be used as a first member in other structs, allowing for pointers to be cast between the two types.
*/
struct _EventHandler {
	void* data;  // user-defined convenience pointer. Allows an event handler or any struct in which one is an initial member to be cast into any pointer type
	char * type;  // A string which can be used for simple type-checking or debugging purposes
	
	// Triggered by top-level state changes
	void (*onEnter)     (EventHandler * handler);
	void (*onExit)      (EventHandler * handler);

	// Triggered strictly when pushed/popped, as opposed to onEnter/onExit, which triggers on any top-level state change
	void (*onPush)      (EventHandler * handler);
	void (*onPop)       (EventHandler * handler);
	
	// Triggered by the main event loop
	void (*onTick)      (EventHandler * handler);
	void (*onDraw)      (EventHandler * handler);

	// Input events, each with an additional argument to forward an SDL_Event
	void (*onKey)       (EventHandler * handler, SDL_Event * e);
	void (*onKeyDown)   (EventHandler * handler, SDL_Event * e);
	void (*onKeyUp)     (EventHandler * handler, SDL_Event * e);
	void (*onKeyHold)   (EventHandler * handler, SDL_Event * e);
	void (*onMouseMove) (EventHandler * handler, SDL_Event * e);
	void (*onMouseDown) (EventHandler * handler, SDL_Event * e);
	void (*onMouseDrag) (EventHandler * handler, SDL_Event * e);
	void (*onMouseUp)   (EventHandler * handler, SDL_Event * e);
};

struct _GameState {
	EventHandler events;
	GameState * prevState;
	uint32_t enterTime;
	uint32_t duration;
};

/*
   AllocationState - GameState-based memory allocation.
   When created, allocates some memory, and on tick pops from the stack and frees that memory.
   Contains an internal AllocationState stack in order to prevent overpopulating the main state stack, to which new AllocationStates are added.
*/
struct _AllocationState {
	GameState state;
	int length;
	AllocationState * prev_allocation;
};

typedef struct _StateContainerState {
	GameState state;
	GameState * value;
} StateContainerState;

typedef struct _ActionQueue {
	char * type;
	struct _ActionQueue * next;

	uint32_t start;
	int duration;
} ActionQueue;


// Convenience casting macros
#define EventHandler(H) ((EventHandler *) H)
#define GameState(S) ((GameState *) S)
#define ActionQueue(Q) ((ActionQueue *) Q)

int initGame();
GameState * gamePushState(GameState * state);
GameState * gamePopState();
void gameMainLoop();

EventHandler * makeEventHandler();
GameState * makeGameState();

ActionQueue * makeAction(char * type);
ActionQueue * pushAction(char * type);
int pollAction(char * type);
void nextAction();

void blit(SDL_Texture * texture, SDL_Rect * src, SDL_Rect * dest);

uint32_t stateUpdateTime(GameState * state, uint32_t time);

void onTick(EventHandler * h);
void onDraw(EventHandler * h);
void onKeyUp(EventHandler * h, SDL_Event * e);
void onKeyDown(EventHandler * h, SDL_Event * e);
void onMouseUp(EventHandler * h, SDL_Event * e);
void onMouseDown(EventHandler * h, SDL_Event * e);

void prevStateOnDraw(EventHandler * h);
void popEvent(EventHandler * h);
void freeEvent(EventHandler *);

void allocationStateOnPop(EventHandler * h);
void * gameCalloc(int size, int n);

GameState * gamePushStateOnTick(GameState * state);
void containerStateReplace(EventHandler * h);

#endif
