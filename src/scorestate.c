#include "sprites.h"
#include "scorestate.h"
#include "stateengine.h"
#include "hunter.h"
#include "score.h"

ScoreState * makeScoreState(ScoreState * state, MatchContext * match){
	if(state == NULL)
		state = (ScoreState *) calloc(sizeof(ScoreState), 1);
	state->match = match;
	EventHandler(state)->type = "ScoreState";
	EventHandler(state)->onDraw = scoreStateOnDraw;
	EventHandler(state)->onKeyUp = scoreStateOnKeyUp;
	return state;
}

void scoreStateOnDraw(EventHandler * h){
	ScoreState * state = (ScoreState *) h;
	MatchContext * match = state->match;

	for(int h = 0; h < PLAYERS_LENGTH; h++){
		int number = totalScore(match->scores[h]);
		for(
			int place = 0;
			(place == 0) || (number > 0);
			number /= 10, place++
		){
			drawBigNumber(
					100 - place * textures.statbox.w,
					 64 + h * textures.statbox.h,
					number % 10
				);
		}
	}
}

void scoreStateOnKeyUp(EventHandler * h, SDL_Event * e){
	switch(e->key.keysym.scancode){
		case SDL_SCANCODE_RETURN:
		case SDL_SCANCODE_SPACE:
		case SDL_SCANCODE_ESCAPE:
			gamePopState();
			break;
		default:
			break;
	}
}
