#ifndef __dicestate_h
#define __dicestate_h

#include "stateengine.h"
#include "hunter.h"
#include "draw.h"

typedef struct _DiceState {
	GameState state;
	MatchContext * match;
	int num;
	int x, y;
} DiceState;

#define DiceState(d) ((DiceState*) d)

DiceState * initDiceState(DiceState * state, MatchContext * match, int num, int x, int y);
void diceStateOnTick(EventHandler * e);
void diceStateOnDraw(EventHandler * e);

#endif
