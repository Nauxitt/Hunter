#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hunter.h"
#include "hunter_enqueue.h"
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

	enqueueBeginMatchAction(context);
	matchQueueUpdate(context);
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

			enqueueStartTurnAction(context, actor);
			break;

		case TURN_START_ACTION:
			enqueueDrawCardAction(context, actor);
			enqueuePollTurnAction(context, actor);
			enqueueEndTurnAction(context, actor);
			break;

		case TURN_END_ACTION:
			hunterClearBonus(actor);

			// Cycle turns
			if(context->characters[++context->active_player] == NULL)
				context->active_player = 0;
			
			actor = context->characters[context->active_player];
			enqueueStartTurnAction(context, actor);
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

		case DAMAGE_ACTION:
			actor->base_stats.hp -= action->value;
			if(actor->base_stats.hp < 0)
				actor->base_stats.hp = 0;
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

		case DEATH_CHECK_ACTION:
			if(actor->stats.hp <= 0){
				actor->stats.hp = 0;
				enqueueTeleportRandomAction(context, actor);
			}
			break;

		case TELEPORT_RANDOM_ACTION:
			getRandomTile(context, &action->x, &action->y);
			enqueueTeleportAction(
					context, actor, action->x, action->y
				);
			break;

		case TELEPORT_ACTION:
		case MOVE_STEP_ACTION:
			context->map[
					actor->x + context->map_w * actor->y
				].hunter = NULL;

			actor->x = action->x;
			actor->y = action->y;

			context->map[
					actor->x + context->map_w * actor->y
				].hunter = actor;

			break;

		case END_MOVE_ACTION:
			// Open crate if one has been landed on
			if(strcmp(actor->type, "hunter") == 0){
				crate = getCrateAt(context, actor->x, actor->y);
				if(crate != NULL)
					enqueueOpenCrateAction(context, crate, actor);
			}

			// If a hunter ends a moves next to an enemy, poll for attacking
			if(
				getHunterAt(context, actor->x  , actor->y-1) ||
				getHunterAt(context, actor->x  , actor->y+1) ||
				getHunterAt(context, actor->x-1, actor->y)   ||
				getHunterAt(context, actor->x+1, actor->y)
			  )
				enqueuePollCombatAction(context, actor);

			// Exit tile handling
			if(hunterAt(actor, context->exit_x, context->exit_y)){
				// TODO: End game if it's a hunter with the target item
				// Otherwise teleport to random valid location
				enqueueTeleportRandomAction(context, actor);
			}

			break;

		case ROLL_DICE_ACTION:
			rollDice(context);
			break;

		case MOVE_ROLL_BONUS_ACTION:
			action->actor->turn_stats.mov += context->dice[0];
			break;

		case CATCH_ROLL_BONUS_ACTION:
			action->actor->turn_stats.mov += context->dice_total;
			break;

		case ESCAPE_ROLL_BONUS_ACTION:
			action->actor->turn_stats.mov += context->dice_total2;
			break;

		case ATTACK_ROLL_BONUS_ACTION:
			action->actor->turn_stats.atk += context->dice_total;
			break;

		case DEFENSE_ROLL_BONUS_ACTION:
			action->actor->turn_stats.def += context->dice_total2;
			break;

		case REMOVE_RELIC_ACTION:
			break;

		case USE_CARD_ACTION:
			hunterUseCard(context, actor, action->card);
			break;

		case COMBAT_ACTION:
			// Skip combat if there's no target
			if(action->target == NULL)
				break;

			// General structure of a combat round
			enqueueEnterCombatAction(context, actor, action->target);
			enqueuePollDefenderAction(context, action->target);
			enqueuePollAttackerCardAction(context, actor);
			enqueueExecuteCombatAction(context);
			enqueueExitCombatAction(context);

			// If anyone died, teleport them to a random location
			enqueueDeathCheckAction(context, actor);
			enqueueDeathCheckAction(context, action->target);
			break;

		case ENTER_COMBAT_ACTION:
			context->attacker = action->actor;
			context->defender = action->target;
			break;

		case EXIT_COMBAT_ACTION:
			context->attacker = NULL;
			context->defender = NULL;
			context->attacker_card = NULL;
			context->defender_card = NULL;
			context->defender_action = NULL;
			break;

		case EXECUTE_COMBAT_ACTION:
			if(context->defender_action->type == SURRENDER_ACTION){
				enqueueRemoveRelicAction(
						context,
						context->defender,
						context->defender_action->relic
					);
				enqueueGiveRelicAction(
						context,
						context->attacker,
						context->defender_action->relic
					);

				// --- Teleport after combat ---

				MatchAction * insert = context->action;

				// Find the end of combat
				while(insert->next && insert->type != EXIT_COMBAT_ACTION)
					insert = insert->next;
				
				// Make and insert teleportation action
				MatchAction * new = (MatchAction*) calloc(sizeof(MatchAction), 1);
				new->type = TELEPORT_RANDOM_ACTION;
				new->actor = context->defender;
				new->next = insert->next;
				insert->next = new;
			}
			else if(context->defender_action->type == ESCAPE_ACTION){
				enqueueEscapeAttemptAction(context);
			}
			else {
				// A defend action will double the user's base DEF stat.
				if(context->defender_action->type == DEFEND_ACTION){
					context->defender->turn_stats.def += context->defender->base_stats.def/2;
				}

				if(context->attacker_card)
					enqueueUseCardAction(context, context->attacker, context->attacker_card);

				if(context->defender_card)
					enqueueUseCardAction(context, context->defender, context->defender_card);

				enqueueAttackAction(context, context->attacker, context->defender);

				if(context->defender_action->type == ATTACK_ACTION){
					enqueueAttackAction(context, context->defender, context->attacker);
				}

			}
			break;

		case DEFEND_ACTION:
		case ESCAPE_ACTION:
		case SURRENDER_ACTION:
			/*
			   These actions are not directly handled by matchCycle or the action stack.
			*/
			break;

		case ATTACK_ACTION:
			enqueueRollDiceAction(context);
			break;

		case OPEN_CRATE_ACTION:
			enqueueGiveRelicAction(context, actor, action->crate->contents);
			action->crate->exists = 0;
			break;

		case GIVE_RELIC_ACTION:
			// TODO: check if hunter successfully received the item, or if we need to poll for whether they wish to discard another item to make room.
			hunterAddRelic(actor, action->relic);
			break;

		case POLL_MOVE_CARD_ACTION:
		case POLL_MOVE_ACTION:
		case POLL_ATTACK_ACTION:
		case POLL_COMBAT_CARD_ACTION:
		case POLL_COMBAT_ACTION:
		case POLL_DEFEND_ACTION:
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
		case POLL_DEFEND_ACTION:
			context->polling = 1;
			break;

		default:
			printMatchAction(action);
			context->polling = 0;
			break;
	}
	
	if(context->polling == 0)
		free(action);
}

void printMatchQueue(MatchContext * context){
	printf("Match queue");
	if(context->polling)
		printf(" (polling)");
	printf(":\n");

	for(MatchAction * action = context->enqueue; action; action = action->next){
		printf("\t(Enqueued) ");
		printMatchAction(action);
	}
	for(MatchAction * action = context->action; action; action = action->next){
		printf("\t");
		printMatchAction(action);
	}
}

void printMatchAction(MatchAction * action){
	printf("%s(", getMatchActionName(action->type));

	switch(action->type){
		// For these actions, print nothing inside the parenthesis, as either printing their contents is unimplemented or they do not use any parameters
		case DAMAGE_ACTION:
		case EXIT_COMBAT_ACTION:
		case REMOVE_RELIC_ACTION:
		case ROLL_DICE_ACTION:
		case BEGIN_MATCH_ACTION:
		case EXECUTE_COMBAT_ACTION:
		case DEFEND_ACTION:
		case ESCAPE_ACTION:
		case ATTACK_ACTION:
		case SURRENDER_ACTION:
			break;
		
		// Actions in which a single hunter is the only parameter
		case TURN_START_ACTION:
		case TURN_END_ACTION:
		case DRAW_CARD_ACTION:
		case HEAL_ACTION:
		case POLL_TURN_ACTION:
		case POLL_MOVE_CARD_ACTION:
		case POLL_MOVE_ACTION:
		case END_MOVE_ACTION:
		case REST_ACTION:
		case DEATH_CHECK_ACTION:
		case TELEPORT_RANDOM_ACTION:
		case POLL_DEFEND_ACTION:
		case MOVE_ROLL_BONUS_ACTION:
		case CATCH_ROLL_BONUS_ACTION:
		case ESCAPE_ROLL_BONUS_ACTION:
		case ATTACK_ROLL_BONUS_ACTION:
		case DEFENSE_ROLL_BONUS_ACTION:
		case POLL_COMBAT_ACTION:
		case POLL_ATTACK_ACTION:
		case POLL_COMBAT_CARD_ACTION:
			printf("%s", action->actor->name);
			break;

		// Actions where one hunter targets another
		case COMBAT_ACTION:
		case ENTER_COMBAT_ACTION:
			printf("%s, %s", action->actor->name, action->target ? action->target->name : NULL);
			break;

		// Actions with a special configuration of parameters
		case TELEPORT_ACTION:
		case MOVE_ACTION:
		case MOVE_STEP_ACTION:
			printf("%s, [%d, %d]", action->actor->name, action->x, action->y);
			break;
		
		case USE_CARD_ACTION:
			// TODO: print card
			printf("%s", action->actor->name);
			break;

		case GIVE_RELIC_ACTION:
			printf("%s, %s", action->actor->name, action->relic->name);
			break;

		case OPEN_CRATE_ACTION:
			printf("%s, [%d, %d]", action->actor->name, action->crate->x, action->crate->y);
			break;
	}

	printf(")\n");
}

void getRandomTile(MatchContext * context, int * x, int * y){ // TODO
	*x = 0;
	*y = 0;
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

Hunter * getHunterAt(MatchContext * context, int x, int y){
	for(int n=0; n < 4; n++){
		Hunter * hunter = context->characters[n];
		if(hunter->x != x) continue;
		if(hunter->y != y) continue;
		return hunter;
	}
	return NULL;
}

uint8_t hunterAt(Hunter * hunter, int x, int y){
	return (hunter->x == x) && (hunter->y == y);
}

void hunterUseCard(MatchContext * context, Hunter * hunter, Card * card){
	Hunter * opponent = hunter == context->attacker ? context->attacker : context->defender;

	switch(card->type){
		case MOVE_CARD:
			hunter->turn_stats.mov += card->num;
			break;

		case MOVE_EXIT_CARD:
			if(context->defender)
				; // TODO: enqueueEscapeCombatAction();
			else
				enqueueTeleportRandomAction(context, hunter);
			break;

		case ATTACK_CARD:
			hunter->turn_stats.atk += card->num;

		case ATTACK_DOUBLE_CARD:
			hunter->turn_stats.atk += hunter->base_stats.atk;

		case ATTACK_COPY_CARD:
			hunter->turn_stats.atk += opponent->base_stats.atk;
			break;

		default:
			break;
	}
}

int hunterInventoryLength(Hunter * hunter){
	int len = 0;
	for(; len < INVENTORY_LIMIT; len++)
		if(hunter->inventory[len] == NULL)
			return len;
	
	return INVENTORY_LIMIT;
}

int hunterAddRelic(Hunter * hunter, Relic * relic){
	int len = hunterInventoryLength(hunter);

	if(len == INVENTORY_LIMIT)
		return 0;
	
	hunter->inventory[len] = relic;
	return 1;
}

void rollDice(MatchContext * context){
	for(int x=0; x < 4; x++)
		context->dice[x] = rand() % 6 + 1;
	context->dice_total = context->dice[0] + context->dice[1];
	context->dice_total2 = context->dice[2] + context->dice[3];
}

uint8_t postCombatAction(MatchContext * context, Hunter * attacker, Hunter * defender){
	if(attacker == NULL)
		attacker = context->characters[context->active_player];

	// Don't post an action if we're not polling
	if(context->polling == 0)
		return 1;

	// Check if we're polling for this type of action
	if(
			(context->action->type != POLL_TURN_ACTION)   && 
			(context->action->type != POLL_COMBAT_ACTION)
	  )
		return 1;

	enqueueCombatAction(context, attacker, defender);
	return 0;
}

uint8_t postDefenderAction(MatchContext * context, enum MatchActionType type, Card * card){
	if(context->polling == 0)
		return 1;

	if(context->action->type != POLL_DEFEND_ACTION)
		return 1;
	
	MatchAction * new = (MatchAction*) calloc(sizeof(MatchAction), 1);

	switch(type){
		case SURRENDER_ACTION:
			if(card != NULL)
				return 1;
		case ATTACK_ACTION:
		case DEFEND_ACTION:
		case ESCAPE_ACTION:
			break;

		default:
			free(new);
			return 1;
	}
	new->type = type;
	new->actor = context->defender;
	new->card = card;

	context->defender_action = new;

	context->polling = 0;
	MatchAction * a = context->action;
	context->action = a->next;
	free(a);
	return 0;
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
			enqueueRollDiceAction(context);
			enqueueMoveRollBonusAction(context, character);
			enqueuePollMoveAction(context, character);
			break;

		case REST_ACTION:
			enqueueRestAction(context, character);
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
		case DAMAGE_ACTION: return "DAMAGE_ACTION";
		case POLL_TURN_ACTION: return "POLL_TURN_ACTION";
		case POLL_MOVE_CARD_ACTION: return "POLL_MOVE_CARD_ACTION";
		case POLL_MOVE_ACTION: return "POLL_MOVE_ACTION";
		case POLL_COMBAT_ACTION: return "POLL_COMBAT_ACTION";
		case POLL_ATTACK_ACTION: return "POLL_ATTACK_ACTION";
		case POLL_COMBAT_CARD_ACTION: return "POLL_COMBAT_CARD_ACTION";
		case ROLL_DICE_ACTION: return "ROLL_DICE_ACTION";
		case MOVE_STEP_ACTION: return "MOVE_STEP_ACTION";
		case MOVE_ACTION: return "MOVE_ACTION";
		case END_MOVE_ACTION: return "END_MOVE_ACTION";
		case ATTACK_ACTION: return "ATTACK_ACTION";
		case REST_ACTION: return "REST_ACTION";
		case DEFEND_ACTION: return "DEFEND_ACTION";
		case ESCAPE_ACTION: return "ESCAPE_ACTION";
		case SURRENDER_ACTION: return "SURRENDER_ACTION";
		case OPEN_CRATE_ACTION: return "OPEN_CRATE_ACTION";
		case GIVE_RELIC_ACTION: return "GIVE_RELIC_ACTION";
		case POLL_DEFEND_ACTION: return "POLL_DEFEND_ACTION";
		case TELEPORT_ACTION: return "TELEPORT_ACTION";
		case TELEPORT_RANDOM_ACTION: return "TELEPORT_RANDOM_ACTION";
		case ENTER_COMBAT_ACTION: return "ENTER_COMBAT_ACTION";
		case EXIT_COMBAT_ACTION: return "EXIT_COMBAT_ACTION";
		case DEATH_CHECK_ACTION: return "DEATH_CHECK_ACTION";
		case COMBAT_ACTION: return "COMBAT_ACTION";
		case EXECUTE_COMBAT_ACTION: return "EXECUTE_COMBAT_ACTION";
		case MOVE_ROLL_BONUS_ACTION: return "MOVE_ROLL_BONUS_ACTION";
		case CATCH_ROLL_BONUS_ACTION: return "CATCH_ROLL_BONUS_ACTION";
		case ESCAPE_ROLL_BONUS_ACTION: return "ESCAPE_ROLL_BONUS_ACTION";
		case ATTACK_ROLL_BONUS_ACTION: return "ATTACK_ROLL_BONUS_ACTION";
		case DEFENSE_ROLL_BONUS_ACTION: return "DEFENSE_ROLL_BONUS_ACTION";
		case REMOVE_RELIC_ACTION: return "REMOVE_RELIC_ACTION";
	}
	return NULL;
}
