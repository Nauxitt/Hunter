#ifndef __dicestate_h
#define __dicestate_h

#include "stateengine.h"
#include "hunter.h"
#include "draw.h"

enum DiceColor {
	MOVE_DICE_COLOR,
	DAMAGE_DICE_COLOR,
	DEFENSE_DICE_COLOR,
	HEAL_DICE_COLOR
};

typedef struct _DiceState {
	GameState state;
	int num;
	int x, y;
	enum DiceColor color;
} DiceState;

#define DiceState(d) ((DiceState*) d)

DiceState * initDiceState(DiceState * state, int num, int x, int y, enum DiceColor color);
void diceStateOnTick(EventHandler * e);
void diceStateOnDraw(EventHandler * e);

#endif
