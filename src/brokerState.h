#ifndef __brokerState_h
#define __brokerState_h

#include "stateengine.h"
#include "sprites.h"
#include "draw.h"
#include "menubar.h"
#include "scorestate.h"

typedef struct _BrokerState {
	GameState state;
	MenubarState * menubar;
} BrokerState;

#define BrokerState(s) ((BrokerState*) s)

BrokerState * makeBrokerState(BrokerState * state);
void brokerStateOnDraw(EventHandler * h);
void brokerStateOnKeyUp(EventHandler * h, SDL_Event * e);

#endif
