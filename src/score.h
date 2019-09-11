/*
   relic.h
   
   Contains structs used for handling scores, and handling how points are awarded.
*/

#ifndef __score_h
#define __score_h

typedef struct _Score Score;
typedef struct _Scoreset Scoreset;
typedef struct _ScoringContext ScoringContext;

#include "hunter.h"

typedef struct _Score {
	int cumulative;
	int value;
} Score;

typedef struct _Scoreset {
	Score level_handicap;
	Score relic_bonus;
	Score movement_bonus;
	Score damage_bonus;
	Score flag_bonus;
} Scoreset;

typedef struct _ScoringContext {
	int level_handicap;
	int relic_award;         // pts for each discovered relic held at match end
	int target_relic_award;  // pts for ending with target relic
	int movement_award;      // pts per tile
	int damage_award;        // pts per damage dealt
	int flag_award;          // pts per flag captured
} ScoringContext;

extern ScoringContext DEFAULT_SCORING_CONTEXT;

void scoresetAdd(Scoreset * dest, Scoreset * add);
void matchActionAssignScore(ScoringContext * scoring_context, MatchAction * action);
int totalScore(Scoreset * score);

#endif
