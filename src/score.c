#include "stdlib.h"
#include "score.h"
#include "hunter.h"

ScoringContext DEFAULT_SCORING_CONTEXT = {
	.relic_award = 1000,
	.target_relic_award = 5000,
	.movement_award = 10
};

void scoresetAdd(Scoreset * dest, Scoreset * add){
	dest->relic_bonus.value += add->relic_bonus.value;
	dest->movement_bonus.value += add->movement_bonus.value;
}

void matchActionAssignScore(ScoringContext * scoring_context, MatchAction * action){
	Scoreset * score = (Scoreset *) calloc(sizeof(Scoreset), 1);

	switch(action->type){
		case MOVE_STEP_ACTION:
			score->movement_bonus.value += scoring_context->movement_award;
			break;

		/* TODO: implement scoring for these
		case DEAL_DAMAGE_ACTION:
		case MATCH_END_ACTION:
		case GET_FLAG_ACTION:
		*/

		default:
			free(score);
			return;
	}
	
	action->score = score;
}

int totalScore(Scoreset * score){
	return
		score->relic_bonus.value +
		score->movement_bonus.value;
}
