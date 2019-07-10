#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hunter.h"
#include "cards.h"

void hunterClearBonus(Hunter * h){
	h->turn_stats.atk = 0;
	h->turn_stats.def = 0;
	h->turn_stats.mov = 0;
}

Statset * hunterStats(Hunter * h){
	h->stats.atk = h->base_stats.atk + h->turn_stats.atk;
	h->stats.def = h->base_stats.def/2 + h->turn_stats.def;
	h->stats.mov = h->base_stats.mov/3 + h->turn_stats.mov;
	h->stats.hp = h->base_stats.hp;

	// TODO: max HP penalties from respawn
	h->stats.max_hp = h->base_stats.max_hp;
	return &h->stats;
}

Card * hunterPopCard(Hunter * h, int card_num){
	Card * ret = h->hand[card_num];

	// Shift cards downward
	for(;card_num < HAND_LIMIT - 1; card_num++)
		h->hand[card_num] = h->hand[card_num+1];

	// Zero-terminate
	h->hand[card_num] = NULL;
	return ret;
}

int hunterHandSize(Hunter * h){
	int hand_size = 0;
	while((hand_size < HAND_LIMIT) && (h->hand[hand_size] != NULL)) hand_size++;
	return hand_size;
}

void initMatch(MatchContext * context){
	// Copy deck into a cardpool, from which we will randomly initialize the context's deck
	Card * cardpool[DECK_SIZE] = {};
	for(int n = 0; n < DECK_SIZE; n++)
		cardpool[n] = &CARDS[n];

	for(int cardpool_end = DECK_SIZE-1; cardpool_end >= 0; cardpool_end--){
		// Move a random card from cardpool to deck
		int card_index = rand() % (cardpool_end + 1);
		context->deck[cardpool_end] = cardpool[card_index];

		// Fill in the empty space of the removed card, using the final available element from the cardpool
		cardpool[card_index] = cardpool[cardpool_end];
	}

	context->deck_len = DECK_SIZE;
	context->active_player = 0;

	enqueueBeginMatch(context);
	matchQueueUpdate(context);
}

MatchAction * matchEnqueueAction(MatchContext * context, enum MatchActionType type){
	MatchAction * ret = (MatchAction*) calloc(sizeof(MatchAction), 1);
	ret->type = type;

	// Append new action to enqueue
	MatchAction * end = context->enqueue;
	if(end == NULL)
		context->enqueue = ret;
	else {
		// Append ret to the end of enqueue
		while(end->next != NULL)
			end = end->next;
		end->next = ret;
	}
	return ret;
}

MatchAction * matchEnqueueActorAction(MatchContext * context, enum MatchActionType type, Hunter * actor){
	MatchAction * action = matchEnqueueAction(context, type);
	action->actor = actor;
	return action;
}

void enqueueBeginMatch(MatchContext * context){
	matchEnqueueAction(context, BEGIN_MATCH_ACTION);
}

void enqueueEndTurn(MatchContext * context, Hunter * actor){
	matchEnqueueActorAction(context, TURN_END_ACTION, actor);
}

void enqueueStartTurn(MatchContext * context, Hunter * actor){
	matchEnqueueActorAction(context, TURN_START_ACTION, actor);
}

void enqueueRestAction(MatchContext * context, Hunter * actor){
	matchEnqueueActorAction(context, REST_ACTION, actor);
}

void enqueueUseCardAction(MatchContext * context, Hunter * actor, Card * card){
	MatchAction * action = matchEnqueueActorAction(context, USE_CARD_ACTION, actor);
	action->card = card;
}

void enqueueRollMoveDiceAction(MatchContext * context, Hunter * actor){
	MatchAction * action = matchEnqueueActorAction(context, ROLL_MOVE_DICE_ACTION, actor);
	action->value = 1;  // Roll only one dice
}

void enqueueMoveAction(MatchContext * context, Hunter * actor, int x, int y){
	MatchAction * action = matchEnqueueActorAction(context, MOVE_ACTION, actor);
	action->x = x;
	action->y = y;
}

void enqueueMoveStepAction(MatchContext * context, Hunter * actor, int x, int y){
	MatchAction * action = matchEnqueueActorAction(context, MOVE_STEP_ACTION, actor);
	action->x = x;
	action->y = y;
}

void enqueueEndMoveAction(MatchContext * context, Hunter * actor){
	matchEnqueueActorAction(context, END_MOVE_ACTION, actor);
}

void enqueuePollTurnAction(MatchContext * context, Hunter * actor){
	matchEnqueueActorAction(context, POLL_TURN_ACTION, actor);
}

void enqueuePollMoveAction(MatchContext * context, Hunter * actor){
	matchEnqueueActorAction(context, POLL_MOVE_ACTION, actor);
}

void enqueueDrawCard(MatchContext * context, Hunter * hunter){
	matchEnqueueActorAction(context, DRAW_CARD_ACTION, hunter);
}

void enqueueDrawCards(MatchContext * context, Hunter * hunter, int number){
	for(int n=0; n < number; n++)
		enqueueDrawCard(context, hunter);
}

void enqueueHealAction(MatchContext * context, Hunter * actor, int amount){
	MatchAction * action = matchEnqueueActorAction(context, HEAL_ACTION, actor);
	action->value = amount;
}

void enqueueOpenCrateAction(MatchContext * context, Crate * crate, Hunter * actor){
	MatchAction * action = matchEnqueueActorAction(context, OPEN_CRATE_ACTION, actor);
	action->crate = crate;
}

void matchQueueUpdate(MatchContext * context){
	// Push enqueue onto stack
	if(context->enqueue){
		MatchAction * end = context->enqueue;
		while(end->next != NULL)
			end = end->next;

		end->next = context->action;
		context->action = context->enqueue;
		context->enqueue = NULL;
	}
}

int matchQueueLength(MatchContext * context){
	int n = 0;
	for(MatchAction * a = context->action; a; a = a->next) n++;
	return n;
}


void matchCycle(MatchContext * context){
	/*
	   This function serves as an iteration in the game's sequential, turn-based logic, and maintains a match's gameplay mechanics. Running the function once pops an action from the match's action stack, and responds to that action by modifying gameplay variables, as well as the context's action stack.
	*/

	MatchAction * action = context->action;
	context->action = action->next;

	if(context->polling != 0 && context->enqueue != NULL){
		free(action);
		matchQueueUpdate(context);
		context->polling = 0;
		action = context->action;
		context->action = action->next;
	}

	Hunter * actor = action->actor;
	if(actor == NULL)
		actor = context->characters[context->active_player];

	Statset * active_stats = hunterStats(actor);

	Crate * crate;

	switch(action->type){
		case BEGIN_MATCH_ACTION:
			// Deal each player four cards
			for(int n = 0; n < 4; n++)
				enqueueDrawCards(context, context->characters[n], 4);

			enqueueStartTurn(context, actor);
			break;

		case TURN_START_ACTION:
			enqueueDrawCard(context, actor);
			enqueuePollTurnAction(context, actor);
			enqueueEndTurn(context, actor);
			break;

		case TURN_END_ACTION:
			hunterClearBonus(actor);

			// Cycle turns
			if(context->characters[++context->active_player] == NULL)
				context->active_player = 0;
			
			actor = context->characters[context->active_player];
			enqueueStartTurn(context, actor);
			break;

		case DRAW_CARD_ACTION:
			if(context->deck_len){
				int hand_size = hunterHandSize(actor);
				if(hand_size < HAND_LIMIT && context->deck_len){
					actor->hand[hand_size] = context->deck[--context->deck_len];
				}
			}
			break;

		case REST_ACTION:
			enqueueHealAction(context, actor, active_stats->max_hp/4);
			enqueueDrawCards(context, actor, 2);
			break;

		case HEAL_ACTION:
			actor->base_stats.hp += action->value;
			if(actor->base_stats.hp > actor->stats.max_hp)
				actor->base_stats.hp = actor->stats.max_hp;
			break;

		case MOVE_ACTION:
			for(int x = actor->x; x != action->x;){
				x += (x > action->x) ? -1 : 1;
				enqueueMoveStepAction(context, actor, x, actor->y);
			}
			for(int y = actor->y; y != action->y;){
				y += (y > action->y) ? -1 : 1;
				enqueueMoveStepAction(context, actor, action->x, y);
			}
			enqueueEndMoveAction(context, actor);
			break;

		case MOVE_STEP_ACTION:
			actor->x = action->x;
			actor->y = action->y;
			break;

		case END_MOVE_ACTION:
			// Open crate if one has been landed on
			if(strcmp(actor->type, "hunter") == 0){
				crate = getCrateAt(context, actor->x, actor->y);
				if(crate != NULL)
					enqueueOpenCrateAction(context, crate, actor);
			}

			// TODO: landing on the exit tile
			//     - End game if it's a hunter with the target item
			//     - Otherwise teleport to random valid location

			break;

		case ROLL_MOVE_DICE_ACTION:
			rollDice(context);
			actor->turn_stats.mov += context->dice[0];
			break;

		case ROLL_ATTACK_DICE_ACTION:
		case ROLL_DEFENSE_DICE_ACTION:
			break;

		case USE_CARD_ACTION:
			hunterUseCard(context, actor, action->card);
			break;

		case ATTACK_ACTION:
		case COUNTERATTACK_ACTION:
		case DEFEND_ACTION:
		case ESCAPE_ACTION:
		case SURRENDER_ACTION:
		case OPEN_CRATE_ACTION:
			break;

		case POLL_MOVE_CARD_ACTION:
		case POLL_MOVE_ACTION:
		case POLL_ATTACK_ACTION:
		case POLL_COMBAT_CARD_ACTION:
		case POLL_COMBAT_ACTION:
		case POLL_TURN_ACTION:
			/*
				When polling for an action, if a new action has been posted, push that on top of the stack and continue from there in the next action cycle.  Otherwise, keep this polling action on the stack.
		   */
			if(context->postedAction == NULL){
				context->action = action;    // Return action to stack
			}
			else {
				context->enqueue = context->postedAction;
				context->postedAction = NULL;
			}
			break;
	}
	matchQueueUpdate(context);

	switch(context->action->type){
		case POLL_MOVE_CARD_ACTION:
		case POLL_MOVE_ACTION:
		case POLL_ATTACK_ACTION:
		case POLL_COMBAT_CARD_ACTION:
		case POLL_COMBAT_ACTION:
		case POLL_TURN_ACTION:
			context->polling = 1;
			break;

		default:
			printf("%s: %s\n", getMatchActionName(action->type), action->actor ? action->actor->name: 0);
			context->polling = 0;
			break;
	}
	
	if(context->polling == 0)
		free(action);
}

Crate * getCrateAt(MatchContext * context, int x, int y){
	for(int n=0; n < context->crates_len; n++){
		Crate * crate = &context->crates[n];
		if(crate->x != x) continue;
		if(crate->y != y) continue;
		if(crate->exists)
			return crate;
	}
	return NULL;
}

void hunterUseCard(MatchContext * context, Hunter * hunter, Card * card){
	switch(card->type){
		case MOVE_CARD:
			hunter->turn_stats.mov += card->num;
			break;

		case MOVE_EXIT_CARD:
			// enqueueTeleportAction();
			//    --OR--
			// enqueueEscapeCombatAction();
			break;

		case ATTACK_CARD:
			hunter->turn_stats.atk += card->num;

		case ATTACK_DOUBLE_CARD:
			hunter->turn_stats.atk += hunter->base_stats.atk;

		case ATTACK_COPY_CARD:
			// TODO: figure out how to get the enemy hunter in combat
			break;

		default:
			break;
	}
}

void rollDice(MatchContext * context){
	context->dice[0] = rand() % 6 + 1;
	context->dice[1] = rand() % 6 + 1;
	context->dice_total = context->dice[0] + context->dice[1];
}

uint8_t postTurnAction(MatchContext * context, enum MatchActionType type, Hunter * character, Card * card){
	if(character == NULL)
		character = context->characters[context->active_player];

	// Don't post an action if we're not polling
	if(context->polling == 0)
		return 1;

	// Check if we're polling for this type of action
	if(context->action->type != POLL_TURN_ACTION)
		return 1;

	switch(type){
		case MOVE_ACTION:
			if(card)
				enqueueUseCardAction(context, character, card);
			enqueueRollMoveDiceAction(context, character);
			enqueuePollMoveAction(context, character);
			break;

		case REST_ACTION:
			enqueueRestAction(context, character);
			break;

		case ATTACK_ACTION:
			break;

		default:
			return 1;
	}
	return 0;
}


uint8_t postMoveAction(MatchContext * context, Hunter * character, int x, int y){
	if(character == NULL)
		character = context->characters[context->active_player];

	// Don't post an action if we're not polling
	if(!context->polling)
		return 1;

	// Check if we're polling for this type of action
	if(context->action->type != POLL_MOVE_ACTION)
		return 1;

	context->polling = 0;
	MatchAction * pop = context->action;
	context->action = pop->next;
	free(pop);
	enqueueMoveAction(context, character, x, y);
	return 0;
}

const char * getMatchActionName(enum MatchActionType type){
	switch(type){
		case BEGIN_MATCH_ACTION: return "BEGIN_MATCH_ACTION";
		case TURN_START_ACTION: return "TURN_START_ACTION";
		case TURN_END_ACTION: return "TURN_END_ACTION";
		case DRAW_CARD_ACTION: return "DRAW_CARD_ACTION";
		case USE_CARD_ACTION: return "USE_CARD_ACTION";
		case HEAL_ACTION: return "HEAL_ACTION";
		case POLL_TURN_ACTION: return "POLL_TURN_ACTION";
		case POLL_MOVE_CARD_ACTION: return "POLL_MOVE_CARD_ACTION";
		case POLL_MOVE_ACTION: return "POLL_MOVE_ACTION";
		case POLL_ATTACK_ACTION: return "POLL_ATTACK_ACTION";
		case POLL_COMBAT_CARD_ACTION: return "POLL_COMBAT_CARD_ACTION";
		case ROLL_MOVE_DICE_ACTION: return "ROLL_MOVE_DICE_ACTION";
		case ROLL_ATTACK_DICE_ACTION: return "ROLL_ATTACK_DICE_ACTION";
		case ROLL_DEFENSE_DICE_ACTION: return "ROLL_DEFENSE_DICE_ACTION";
		case POLL_COMBAT_ACTION: return "POLL_COMBAT_ACTION";
		case MOVE_STEP_ACTION: return "MOVE_STEP_ACTION";
		case MOVE_ACTION: return "MOVE_ACTION";
		case END_MOVE_ACTION: return "END_MOVE_ACTION";
		case ATTACK_ACTION: return "ATTACK_ACTION";
		case REST_ACTION: return "REST_ACTION";
		case COUNTERATTACK_ACTION: return "COUNTERATTACK_ACTION";
		case DEFEND_ACTION: return "DEFEND_ACTION";
		case ESCAPE_ACTION: return "ESCAPE_ACTION";
		case SURRENDER_ACTION: return "SURRENDER_ACTION";
		case OPEN_CRATE_ACTION: return "OPEN_CRATE_ACTION";
	}
	return NULL;
}
