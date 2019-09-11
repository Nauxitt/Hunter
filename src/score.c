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
	MatchContext * context = action->context;

	switch(action->type){
		case MOVE_STEP_ACTION:
			score->movement_bonus.value += scoring_context->movement_award;
			break;

		case END_MATCH_ACTION:
			// Relic scores
			for(int n=0; n < PLAYERS_LENGTH; n++){
				Hunter * h = context->characters[n];
				Scoreset * score = context->scores[n];
				
				// Iterate through hunter's inventory
				for(Relic ** slot = (Relic**) &h->inventory; *slot != NULL; slot++){
					// Dont' score items which entered the game with a player
					if((*slot)->player_item)
						continue;

					// Increment score based on whether the relic is the target.
					score->relic_bonus.value += (
							(*slot == context->target_relic) ?
							context->scoring_context->target_relic_award :
							context->scoring_context->relic_award
						);
				}
			}
			break;


		/* TODO: implement scoring for these
		case DEAL_DAMAGE_ACTION:
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
