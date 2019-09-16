#ifndef __scorestate_h
#define __scorestate_h

#include "stateengine.h"
#include "hunter.h"
#include "score.h"

typedef struct _ScoreState {
	GameState state;
	MatchContext * match;
	Hunter * hunters[PLAYERS_LENGTH + 1];
	Score * scores[PLAYERS_LENGTH + 1];
} ScoreState;

ScoreState * makeScoreState(ScoreState * state, MatchContext * match);
void scoreStateOnDraw(EventHandler * h);
void scoreStateOnKeyUp(EventHandler * h, SDL_Event * e);

#endif
