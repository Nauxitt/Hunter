#ifndef __statAllocatorPanel_h
#define __statAllocatorPanel_h

#include "stateengine.h"
#include "draw.h"
#include <SDL2/SDL.h>

#define STAT_ALLOCATOR_W 300
#define STAT_ALLOCATOR_H 200

typedef struct _StatAllocatorState {
	GameState state;
	SDL_Rect rect;
	enum WindowColor color;

	int selector;
	Hunter * hunter;
	Statset stats;
	int points;
} StatAllocatorState;

#define StatAllocatorState(s) ((StatAllocatorState*) s)

StatAllocatorState *  makeStatAllocatorState(StatAllocatorState * state, Hunter * hunter);
void statAllocatorStateSave(StatAllocatorState * state);
void statAllocatorStateOnKeyUp(EventHandler * h, SDL_Event * e);
void statAllocatorStateOnDraw(EventHandler * h);

#endif
