#ifndef __handstate_h
#define __handstate_h

#include "draw.h"
#include "hunter.h"
#include "stateengine.h"

typedef struct _HandState {
	GameState state;
	Hunter * hunter;
	Card ** card_target;
	enum  WindowColor color;
	int selector;
	int x, y;
} HandState;

#define HandState(h) ((HandState*) h)

HandState * makeHandState(HandState * state, Hunter * hunter, int x, int y);
void handOnTick(EventHandler * h);
void handOnDraw(EventHandler * h);
void handOnKeyUp(EventHandler * h, SDL_Event * e);
void handOnMouseUp(EventHandler * h, SDL_Event * e);

#endif
