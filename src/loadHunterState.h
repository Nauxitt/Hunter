#ifndef __loadHunterState_h
#define __loadHunterState_h

#include <SDL2/SDL.h>
#include "stateengine.h"
#include "hunter.h"
#include "draw.h"
#include <sys/stat.h>

struct HunterFileLinkedList {
	char fpath[128];
	struct stat attr;
	
	Hunter hunter;
	
	// double-linked list
	struct HunterFileLinkedList * prev;
	struct HunterFileLinkedList * next;
};

typedef struct _LoadHunterState {
	GameState state;
	SDL_Rect rect;
	enum WindowColor color;

	int selector;
	int scroll;
	int max_rows;

	Hunter * hunter;
	struct HunterFileLinkedList * hunter_list;
} LoadHunterState;

#define LoadHunterState(S) ((LoadHunterState*) S)

LoadHunterState * makeLoadHunterState(LoadHunterState * state, Hunter * hunter);
void loadHunterStateOnDraw(EventHandler * h);
void loadHunterStateOnKeyUp(EventHandler * h, SDL_Event * e);
void loadHunterStateOnPush(EventHandler * h);
void loadHunterStateOnPop(EventHandler * h);
	
#endif
