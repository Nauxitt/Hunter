#ifndef __statboxDisplayState_h
#define __statboxDisplayState_h

#include "stateengine.h"
#include "draw.h"
#include "hunter.h"

typedef struct _StatboxDisplayState {
	GameState state;
	Hunter ** hunters_list;
	enum StatboxViews view;
} StatboxDisplayState;

#define StatboxDisplayState(S) ((StatboxDisplayState *) S)

StatboxDisplayState * makeStatboxDisplayState(StatboxDisplayState * state);
void statboxDisplayStateOnDraw(EventHandler * h);
void statboxDisplayStateOnKeyUp(EventHandler * h, SDL_Event * e);

#endif
