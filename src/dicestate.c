#include "stateengine.h"
#include "hunter.h"
#include "dicestate.h"
#include "sprites.h"
#include "draw.h"
#include "utils.h"

#define DICE_FLIP_DURATION 275
#define DICE_SHOW_DURATION 750
#define DICE_SPIN_DURATION   5

DiceState * initDiceState(DiceState * state, MatchContext * match, int num, int x, int y, enum DiceColor color){
	if(state == NULL)
		state = (DiceState*) calloc(sizeof(DiceState), 1);

	state->match = match;
	state->num = num;
	state->x = x;
	state->y = y;
	state->color = color;

	EventHandler(state)->type = "DiceState";
	EventHandler(state)->onDraw = diceStateOnDraw;
	EventHandler(state)->onTick = diceStateOnTick;

	return state;
}

void diceStateOnTick(EventHandler * h){
	DiceState * state = DiceState(h);
	int duration = GameState(state)->duration;

	if(duration >= DICE_FLIP_DURATION + DICE_SHOW_DURATION){
		gamePopState();
		free(state);
		matchCycle(state->match);
	}
}

void diceStateOnDraw(EventHandler * h){
	prevStateOnDraw(h);

	DiceState * state = DiceState(h);
	int duration = GameState(state)->duration;
	
	if(duration < DICE_FLIP_DURATION){
		float scale = ((DICE_FLIP_DURATION - (float) duration) / DICE_FLIP_DURATION) * 2 + 1;
		float hscale = (float)(duration % DICE_SPIN_DURATION)/(float) DICE_SPIN_DURATION * scale;
		
		SDL_Rect dest = {
				game.w/2 - textures.dice.w/2 * hscale,
				state->y * (2 - scale),
				textures.dice.w * hscale,
				textures.dice.h * scale
			};

		drawDiceBack(&dest);	
	}

	else if(duration < DICE_FLIP_DURATION + DICE_SHOW_DURATION){
		void (*drawDice)(int n, int x, int y) = NULL;

		switch(state->color){
			case MOVE_DICE_COLOR: drawDice = drawMoveDice; break;
			case DAMAGE_DICE_COLOR: drawDice = drawDamageDice; break;
			case DEFENSE_DICE_COLOR: drawDice = drawDefenseDice; break;
			case HEAL_DICE_COLOR: drawDice = NULL; break;
		}

		drawDice(
				state->num,
				state->x - textures.dice.w/2,
				state->y
			);
	} 
}
