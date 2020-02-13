#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hunter.h"
#include "hunter_enqueue.h"
#include "cards.h"
#include "score.h"
#include "ai.h"


void decodeMap(MatchContext * context, char * map_encoded) {
	/*
		Generate a map from a string encoding
		Key:
			space | Empty
			\n    | Map row end
			\0    | Map termination
			#     | Map tile
			C     | Crate
			H     | Hunter
			E     | Exit
	*/

	// Calculate map diminsions
	int x=0, y=0, w=0, h=0;
	for(char * c = map_encoded; *c; c++){
		if(*c == '\n'){
			if(x > w) w = x;
			y++;
			x = -1;
		}
		x++;
	}
	h = y;

	context->map_w = w;
	context->map_h = h;

	Tile * map = (Tile*) calloc(sizeof(Tile), w*h);
	context->map = map;

	// Decode map contents
	Hunter ** hunter_current = (Hunter **) &context->characters;
	Crate * crate_current = context->crates;
	int exit_x, exit_y;

	x = 0;
	y = 0;
	for(char * c = map_encoded; *c; c++){
		Tile * tile = context->map + w*y + x;
		tile->exists = 1;

		tile->x = x;
		tile->y = y;
		tile->path.x = x;
		tile->path.y = y;

		switch(*c){
			case ' ':
				tile->exists = 0;
				break;

			case '#':
				break;

			case 'E':
				exit_x = x;
				exit_y = y;
				break;

			case 'H':
				(*hunter_current)->x = x;
				(*hunter_current)->y = y;
				tile->hunter = *hunter_current++;
				break;

			case 'C':
				crate_current->x = x;
				crate_current->y = y;
				tile->crate = crate_current++;
				break;

			case '\n':
				y++;
				x = -1;
				break;
		}
		x++;
	}

	context->exit_x = exit_x;
	context->exit_y = exit_y;
}

void encodeHunter(Hunter * hunter, char * buffer) {
	int o = 0;  // write offset within buffer
	int data_length = 1;

	// Write hunter data header
	char * header = "HUNTER";
	strcpy(buffer, header);
	int header_len = strlen(header) + 1;
	o += header_len;
	data_length += header_len;

	// Skip writing data length for now
	int length_address = o;
	o += 4;
	data_length += 4;

	// Write level (1)
	buffer[o++] = hunter->level;
	data_length++;

	// Write name (9b)
	strncpy(&buffer[o], (char*) &hunter->name, NAME_MAX_LENGTH);
	o += NAME_MAX_LENGTH+1;
	data_length += NAME_MAX_LENGTH+1;

	// Write base stats (1b/ea.)
	buffer[o++] = hunter->base_stats.mov;
	buffer[o++] = hunter->base_stats.atk;
	buffer[o++] = hunter->base_stats.def;
	buffer[o++] = hunter->base_stats.max_hp;
	buffer[o++] = hunter->base_stats.hp;
	buffer[o++] = hunter->base_stats.restricted_hp;
	data_length += 6;

	// Write credits (4b)
	buffer[o++] = (hunter->credits >> 24) & 0xFF;
	buffer[o++] = (hunter->credits >> 16) & 0xFF;
	buffer[o++] = (hunter->credits >>  8) & 0xFF;
	buffer[o++] =  hunter->credits        & 0xFF;
	data_length += 4;

	// Write inventory (6 slots, 1b/ea.)
	for(int r=0; r < 6; r++)
		if(hunter->inventory[r] == NULL)
			break;
		else
			buffer[o++] = hunter->inventory[r]->item_id;

	data_length += 6;
	
	// Write hunter data length (4b)
	o = length_address;
	buffer[o++] = (data_length >> 24) & 0xFF;
	buffer[o++] = (data_length >> 16) & 0xFF;
	buffer[o++] = (data_length >>  8) & 0xFF;
	buffer[o++] =  data_length        & 0xFF;
}

int decodeHunter(Hunter * hunter, char * buffer) {
	int o = 0;  // write offset within buffer

	// Read hunter data header
	if(strcmp(buffer, "HUNTER") != 0)
		return 1;
	o += strlen(&buffer[o]) + 1;

	// Get data length, ensure it's long enough
	int encoded_length = buffer[o++] << 24;
	encoded_length |= buffer[o++] << 16;
	encoded_length |= buffer[o++] << 8;
	encoded_length |= buffer[o++];

	// Read level (1)
	hunter->level = buffer[o++];

	// Read name (9b)
	strncpy((char*) &hunter->name, &buffer[o], NAME_MAX_LENGTH);
	o += NAME_MAX_LENGTH+1;

	// Read base stats (1b/ea.)
	hunter->base_stats.mov           = buffer[o++];
	hunter->base_stats.atk           = buffer[o++];
	hunter->base_stats.def           = buffer[o++];
	hunter->base_stats.max_hp        = buffer[o++];
	hunter->base_stats.hp            = buffer[o++];
	hunter->base_stats.restricted_hp = buffer[o++];

	// Read credits (4b)
	uint32_t credits = buffer[o++] << 24;
	credits |= buffer[o++] << 16;
	credits |= buffer[o++] <<  8;
	credits |= buffer[o++];
	hunter->credits = credits;

	// Read inventory (6 slots, 1b/ea.)
	// TODO: figure this out
	o += 6;

	// Return error code if the data length is incorrect
	if(o+1 != encoded_length)
		return 1;

	return 0;
}


void printHunter(Hunter * h) {
	hunterStats(h);
	printf(
			"%s (%02u/%02u) (lvl %d) [%u/%u/%u] Cr:%u",
			(char*) h->name,
			h->stats.hp,
			h->stats.max_hp,

			h->level,
			h->base_stats.atk,
			h->base_stats.mov,
			h->base_stats.def,
			h->credits
		);
}

Tile * getTile(MatchContext * context, int x, int y) {
	return &context->map[context->map_w * y + x];
}

void hunterSetPosition(MatchContext * context, Hunter * hunter, int x, int y) {
	// update tile data
	if (context) {
		getTile(context, hunter->x, hunter->y)->hunter = NULL;
		getTile(context, x, y)->hunter = hunter;
	}

	// Update hunter position data
	hunter->x = x;
	hunter->y = y;
}

void hunterClearBonus(Hunter * h) {
	h->turn_stats.atk = 0;
	h->turn_stats.def = 0;
	h->turn_stats.mov = 0;
}

Statset * hunterStats(Hunter * h) {
	if(h == NULL)
		return NULL;

	h->stats.atk = h->base_stats.atk + h->turn_stats.atk;
	h->stats.def = h->base_stats.def/2 + h->turn_stats.def;
	h->stats.mov = h->base_stats.mov/3 + h->turn_stats.mov;
	h->stats.hp = h->base_stats.hp;

	h->stats.restricted_hp = h->base_stats.restricted_hp;
	h->stats.max_hp = h->base_stats.max_hp * 3 + 6 + h->level;
	return &h->stats;
}

Hunter * randomHunter(Hunter * h, int points){
	if(h == NULL)
		h = (Hunter*) calloc(sizeof(Hunter), 1);
	strcpy((char*) &h->type, "hunter");
	
	// Generate a random (and unreadable) name
	for(int n=0; n<5; n++)
		h->name[n] = 65 + rand() % 26;

	h->base_stats.atk = 1;
	h->base_stats.def = 1;
	h->base_stats.mov = 1;
	h->base_stats.max_hp = 1;

	hunterRandomStatIncrease(h, points);

	h->level = points - 10;

	h->base_stats.hp = h->base_stats.max_hp * 3 + 6 + h->level;
	hunterStats(h);

	return h;
}

void hunterRandomStatIncrease(Hunter * h, int points) {
	while(points-- > 0){
		h->level++;
		switch(rand() % 4){
			case 0: h->base_stats.atk++;    break;
			case 1: h->base_stats.def++;    break;
			case 2: h->base_stats.mov++;    break;
			case 3: h->base_stats.max_hp++; break;
		}
	}
}

Card * hunterPopCard(Hunter * h, int card_num) {
	Card * ret = h->hand[card_num];

	// Shift cards downward
	for(;card_num < HAND_LIMIT - 1; card_num++)
		h->hand[card_num] = h->hand[card_num+1];

	// Zero-terminate
	h->hand[card_num] = NULL;
	return ret;
}

int hunterHandSize(Hunter * h) {
	int hand_size = 0;
	while((hand_size < HAND_LIMIT) && (h->hand[hand_size] != NULL)) hand_size++;
	return hand_size;
}

Card * hunterHighestMoveCard(Hunter * h) {
	Card * highest = h->hand[0];

	for (int n=1; n < HAND_LIMIT && h->hand[n]; n++) {
		Card * c = h->hand[n];

		if (c == NULL)
			break;

		switch (c->type) {
			case MOVE_CARD:
				if (c->num > highest->num)
					highest = c;
				break;

			// Exit is the highest card, end search
			case MOVE_EXIT_CARD:
				return c;

			default:
				break;
		}
	}

	return highest;
}

Card * hunterHighestAttackCard(Hunter * h, int copy_val) {
	Card * highest = NULL;
	for (int n=0; n < HAND_LIMIT && h->hand[n]; n++) {
		Card * c = h->hand[n];

		switch (c->type) {
			case ATTACK_CARD:
				if (c->num > highest->num)
					highest = c;
				break;

			case ATTACK_DOUBLE_CARD:
				if (hunterStats(h)->atk > c->num)
					highest = c;
				break;

			case ATTACK_COPY_CARD:
				if (copy_val > highest->num)
					highest = c;
				break;

			default:
				break;
		}
	}

	return highest;
}

void initMatch(MatchContext * context) {
	// Copy deck into a cardpool, from which we will randomly initialize the context's deck
	Card * cardpool[DECK_SIZE] = {};
	for(int n = 0; n < DECK_SIZE; n++)
		cardpool[n] = &DEFAULT_DECK[n];

	for(int cardpool_end = DECK_SIZE-1; cardpool_end >= 0; cardpool_end--){
		// Move a random card from cardpool to deck
		int card_index = rand() % (cardpool_end + 1);
		context->deck[cardpool_end] = cardpool[card_index];

		// Fill in the empty space of the removed card, using the final available element from the cardpool
		cardpool[card_index] = cardpool[cardpool_end];
	}

	context->deck_len = DECK_SIZE;
	context->active_player = 0;

	// Assign hunters numeric id's and max-heal them if their HP is zero
	for(int n=0; n < 4; n++){
		Hunter * hunter = context->characters[n];
		hunter->id = n;

		hunterStats(hunter);

		if (hunter->base_stats.restricted_hp == 0)
			hunter->base_stats.restricted_hp = hunter->stats.max_hp;

		if (hunter->base_stats.hp == 0)
			hunter->base_stats.hp = hunter->base_stats.restricted_hp;

		if (hunter->base_stats.hp < hunter->base_stats.restricted_hp)
			hunter->base_stats.hp = hunter->base_stats.restricted_hp;

		// Update tile/hunter data
		hunterSetPosition(context, hunter, hunter->x, hunter->y);
	}

	// initialize scores
	// TODO: detect whether scoresets are already created
	Scoreset * scores = (Scoreset*) calloc(sizeof(Scoreset), PLAYERS_LENGTH);
	for(int n=0; n < PLAYERS_LENGTH; n++)
		context->scores[n] = scores + n;

	enqueueBeginMatchAction(context);
	matchQueueUpdate(context);
}


void matchQueueUpdate(MatchContext * context) {
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

int matchQueueLength(MatchContext * context) {
	int n = 0;
	for(MatchAction * a = context->action; a; a = a->next) n++;
	return n;
}

void freeAction(MatchAction * a) {
	if(a->score)
		free(a->score);
	free(a);
}

void matchCycle(MatchContext * context) {
	/*
	   This function serves as an iteration in the game's sequential, turn-based logic, and maintains a match's gameplay mechanics. Running the function once pops an action from the match's action stack, and responds to that action by modifying gameplay variables, as well as the context's action stack.
	*/

	MatchAction * action = context->action;

	/*
	   Handle Hunter automated controlling.

	   If the Hunter's controller_hook is set, pass execution to the
	   hook, allowing a chance for new actions to be posted prior to
	   game logic being executed.
   */
	if (		(context->polling) &&
				(action != NULL)   &&
				(action->actor != NULL) &&
				(action->actor->controller_hook)
	){
			action->actor->controller_hook(
					context,
					action->actor,
					action->actor->controller_data
				);
			action = context->action;
	}

	context->action = action->next;

	if(context->polling != 0 && context->enqueue != NULL){
		freeAction(action);
		matchQueueUpdate(context);
		context->polling = 0;
		action = context->action;
		context->action = action->next;
	}

	action->context = context;

	Hunter * actor = action->actor;
	if(actor == NULL)
		actor = context->characters[context->active_player];

	Statset * active_stats = hunterStats(actor);
	Statset * actor_stats;  // TODO: Redundant?
	Statset * target_stats;

	PathNode * path = NULL;

	Crate * crate;

	// Handle scoring, stored in MatchAction
	matchActionAssignScore(context->scoring_context, action);
	if(action->score && action->actor){
		Scoreset * hunterScore = context->scores[action->actor->id];
		scoresetAdd(hunterScore, action->score);
	}

	switch(action->type){
		case POLL_MOVE_CARD_ACTION:
		case POLL_MOVE_ACTION:
		case POLL_ATTACK_ACTION:
		case POLL_COMBAT_CARD_ACTION:
		case POLL_COMBAT_ACTION:
		case POLL_TURN_ACTION:
		case POLL_DEFEND_ACTION:
			break;

		default:
			printMatchAction(action);
			break;
	}

	switch(action->type) {
		case BEGIN_MATCH_ACTION:
			// Deal each player four cards
			for(int n = 0; n < 4; n++)
				enqueueDrawCards(context, context->characters[n], 4);

			enqueueStartTurnAction(context, actor);
			break;

		case END_MATCH_ACTION:
			printf("End match. Scores:\n");
			for(int h=0; h < PLAYERS_LENGTH; h++)
				printf("\t%s: %d\n", context->characters[h]->name, totalScore(context->scores[h]));
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
			if(actor->base_stats.hp > actor->base_stats.restricted_hp) {
				actor->base_stats.hp = actor->base_stats.restricted_hp;
			}
			break;

		case DEAL_DAMAGE_ACTION:
			enqueueDamageAction(
					context, action->target,
					action->value
				);
			break;

		case DAMAGE_ACTION:
			if (action->target->base_stats.hp <= action->value)
				action->target->base_stats.hp = 0;
			else
				action->target->base_stats.hp -= action->value;
			break;

		case MOVE_ACTION:
			path = findPath(context, actor->x, actor->y, action->x, action->y);

			while ((path = path->to)) {
				enqueueMoveStepAction(context, actor, path->x, path->y);
			}

			enqueueEndMoveAction(context, actor);
			break;

		case DEATH_CHECK_ACTION:
			if(actor->base_stats.hp == 0){
				actor->base_stats.restricted_hp /= 2;
				if (actor->base_stats.restricted_hp < 1)
					actor->base_stats.restricted_hp = 1;

				actor->base_stats.hp = actor->base_stats.restricted_hp;
				enqueueTeleportRandomAction(context, actor);
			}
			break;

		case TELEPORT_RANDOM_ACTION:
			getRandomEmptyTile(context, &action->x, &action->y);
			enqueueTeleportAction(
					context, actor, action->x, action->y
				);
			break;

		case TELEPORT_ACTION:
			hunterSetPosition(context, actor, action->x, action->y);
			enqueueEndMoveAction(context, actor);
			break;

		case MOVE_STEP_ACTION:
			hunterSetPosition(context, actor, action->x, action->y);
			break;

		case END_MOVE_ACTION:
			// Open crate if one has been landed on
			crate = getCrateAt(context, actor->x, actor->y);
			if(crate != NULL)
				enqueueOpenCrateAction(context, crate, actor);

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
				// End game if it's a hunter with the target item, 
				// otherwise teleport to random valid location
				
				if(hunterHasRelic(actor, context->target_relic))
					enqueueEndMatchAction(context);
				else
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
			enqueueExecuteCombatAction(context);
			enqueueExitCombatAction(context);
			break;

		case ENTER_COMBAT_ACTION:
			context->attacker = action->actor;
			context->defender = action->target;
			hunterClearBonus(context->attacker);
			hunterClearBonus(context->defender);
			break;

		case EXIT_COMBAT_ACTION:
			// If anyone died, teleport them to a random location
			enqueueDeathCheckAction(context, context->attacker);
			enqueueDeathCheckAction(context, context->defender);

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
				enqueuePollAttackerCardAction(context, actor);

				if(context->defender_card)
					enqueueUseCardAction(context, context->defender, context->defender_card);

				if(context->attacker_card)
					enqueueUseCardAction(context, context->attacker, context->attacker_card);

				enqueueRollDiceAction(context);
				enqueueEscapeAttemptAction(context);  // Potentially skips actions between this and EXIT_COMBAT_ACTION
				enqueueAttackAction(context, context->attacker, context->defender);
			}
			else {
				/*
				   Defending hunter is either blocking or
				   counterattacking.
				*/
				enqueuePollAttackerCardAction(context, actor);

				// A defend action will double the user's base DEF stat.
				if(context->defender_action->type == DEFEND_ACTION){
					enqueueDefendAction(context, context->defender);
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

		case ESCAPE_ACTION:
			for(MatchAction* a = context->action; a->type != EXIT_COMBAT_ACTION; a = context->action){
				context->action = a->next;
				freeAction(a);
			}
			break;

		case ESCAPE_ATTEMPT_ACTION:
			if(
					context->defender->turn_stats.mov + context->dice_total2 >
					context->attacker->turn_stats.mov + context->dice_total
			)
				enqueueEscapeAction(context);
			break;

		case DEFEND_ACTION:
			context->defender->turn_stats.def += context->defender->base_stats.def/2;
			break;

		case SURRENDER_ACTION:
			/*
			   This actions is not directly handled by matchCycle
			   or the action stack.
			*/
			break;

		case ATTACK_ACTION:
			if (active_stats->hp <= 0)
				break;

			enqueueRollDiceAction(context);
			enqueueAttackDamageAction(context, action->actor, action->target);
			break;

		case ATTACK_DAMAGE_ACTION:
			actor_stats = hunterStats(action->actor);
			target_stats = hunterStats(action->target);

			enqueueDealDamageAction(
					context, action->actor, action->target,
					context->dice_total + actor_stats->atk -
					context->dice_total2 - target_stats->def
				);
			break;

		case OPEN_CRATE_ACTION:
			enqueueGiveRelicAction(context, actor, action->crate->contents);
			action->crate->exists = 0;
			break;

		case GIVE_RELIC_ACTION:
			if(hunterAddRelic(actor, action->relic) == 1){
				// TODO: check if hunter successfully received the item, or if we need to poll for whether they wish to discard another item to make room.
				// enqueuePromptReplaceItem
			}
			break;

		case REMOVE_RELIC_ACTION:
			hunterRemoveRelic(actor, action->relic);
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
			// printMatchAction(action);
			context->polling = 0;
			break;
	}
	
	if(context->polling == 0)
		freeAction(action);
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
		case EXIT_COMBAT_ACTION:
		case ROLL_DICE_ACTION:
		case BEGIN_MATCH_ACTION:
		case END_MATCH_ACTION:
		case EXECUTE_COMBAT_ACTION:
		case DEFEND_ACTION:
		case ESCAPE_ACTION:
		case ESCAPE_ATTEMPT_ACTION:
		case SURRENDER_ACTION:
			break;
		
		// Actions in which a single hunter is the only parameter
		case TURN_START_ACTION:
		case TURN_END_ACTION:
		case DRAW_CARD_ACTION:
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
		
		// Actions who's parameters are a hunter and an integer value
		case DAMAGE_ACTION:
		case HEAL_ACTION:
			printf("%s, %d", action->actor->name, action->value);
			break;

		// Actions where one hunter targets another
		case ATTACK_ACTION:
		case COMBAT_ACTION:
		case ENTER_COMBAT_ACTION:
		case ATTACK_DAMAGE_ACTION:
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

		case REMOVE_RELIC_ACTION:
		case GIVE_RELIC_ACTION:
			printf("%s, %s", action->actor->name, action->relic->name);
			break;

		case OPEN_CRATE_ACTION:
			printf("%s, [%d, %d]", action->actor->name, action->crate->x, action->crate->y);
			break;

		case DEAL_DAMAGE_ACTION:
			printf("%s, %s, %d", action->actor->name, action->target->name, action->value);
			break;
	}

	printf(")\n");
}

int pointEmpty(MatchContext * context, int x, int y) {
	return tileEmpty(getTile(context, x, y));
}

int tileEmpty(Tile * tile) {
	if (tile == NULL || !tile->exists)
		return 1;

	return (tile->crate == NULL && tile->hunter == NULL);
}

int pointWalkable(MatchContext * context, int x, int y) {
	// Check map bounds
	if ((x < 0) || (y < 0))
		return 0;

	if ((x >= context->map_w) || (y >= context->map_h))
		return 0;

	return tileWalkable(getTile(context, x, y));
}

int tileWalkable(Tile * tile) {
	if (tile->exists == 0)
		return 0;

	if (tile->hunter != NULL)
		return 0;

	return 1;
}

void getRandomEmptyTile(MatchContext * context, int * x, int * y) {
	int walkable_tiles = 0;
	for(int n=0; n < context->map_h * context->map_w; n++)
		if(context->map[n].exists) walkable_tiles++;

	int target = rand() % walkable_tiles;
	for(int n=0; n < context->map_h * context->map_w; n++) {
		Tile * tile = &context->map[n];
		if (tileWalkable(tile)){
			target--;

			if(target == 0){
				*x = n % context->map_w;
				*y = n / context->map_w;
				return;
			}
		}
	}
}

void getRandomTile(MatchContext * context, int * x, int * y){
	*x = 0;
	*y = 0;

	int tiles = 1;
	for(int n=0; n < context->map_h * context->map_w; n++)
		if(context->map[n].exists) tiles++;

	int target = rand() % tiles;
	for(int n=0; n < context->map_h * context->map_w; n++)
		if(context->map[n].exists){
			target--;

			if(target == 0){
				*x = n % context->map_w;
				*y = n / context->map_w;
				return;
			}
		}
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
	Hunter * opponent = (hunter == context->attacker) ? context->attacker : context->defender;

	// Remove card from hand, if there
	int card_num = -1;
	for(int n=0; n < HAND_LIMIT; n++)
		if(hunter->hand[n] == card){
			card_num = n;
			break;
		}

	if(card_num >= 0)
		hunterPopCard(hunter, card_num);

	// Perform card action
	switch(card->type){
		case MOVE_CARD:
			hunter->turn_stats.mov += card->num;
			break;

		case MOVE_EXIT_CARD:
			if(context->defender)
				; // TODO: enqueueEscapeCombatAction();
			else
				enqueueTeleportAction(context, hunter, context->exit_x, context->exit_y);
			break;

		case ATTACK_CARD:
			hunter->turn_stats.atk += card->num;

		case ATTACK_DOUBLE_CARD:
			hunter->turn_stats.atk += hunter->base_stats.atk;

		case ATTACK_COPY_CARD:
			if (opponent == NULL)
				break;

			hunter->turn_stats.atk += opponent->base_stats.atk;
			break;

		default:
			break;
	}
}

int hunterHasRelic(Hunter * hunter, Relic * relic){
	for(Relic ** slot = (Relic**) &hunter->inventory; *slot; slot++)
		if(*slot == relic)
			return 1;

	return 0;
}

Hunter * getHunterWithTarget(MatchContext * context) {
	for (int n=0; n < PLAYERS_LENGTH; n++) {
		Hunter * h = context->characters[n];
		if (hunterHasRelic(h, context->target_relic))
			return h;
	}

	// No hunter is holding the target relic
	return NULL;
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

Relic * hunterRemoveRelicAt(Hunter * hunter, int index){
	Relic * ret = hunter->inventory[index];
	int n;

	// Shift items back, overwriting removed relic
	for(n = index + 1; n < INVENTORY_LIMIT; n++){
		hunter->inventory[n-1] = hunter->inventory[n];
		if(hunter->inventory[n] == NULL)
			break;
	}

	// Ensure zero-indexing of a previously-full inventory.
	hunter->inventory[INVENTORY_LIMIT - 1] = NULL;

	return ret;
}

int hunterRemoveRelic(Hunter * hunter, Relic * relic){
	int index = -1;
	
	// Search hunter's inventory for relic
	for(int n = 0; n < INVENTORY_LIMIT; n++)
		if(hunter->inventory[n] == relic){
			index = n;
			break;
		}

	// Error if not found
	if(index == -1)
		return 1;
	
	hunterRemoveRelicAt(hunter, index);
	return 0;
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

uint8_t postAttackerCard(MatchContext * context, Card * card){
	if(context->polling == 0)
		return 1;

	if(context->action->type != POLL_ATTACK_ACTION)
		return 1;
	
	context->attacker_card = card;
	
	// Match action stack remove polling action and cleanup
	context->polling = 0;
	MatchAction * a = context->action;
	context->action = a->next;
	freeAction(a);
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

		case ESCAPE_ACTION:
		case ESCAPE_ATTEMPT_ACTION:
		case ATTACK_ACTION:
		case DEFEND_ACTION:
			break;

		default:
			freeAction(new);
			return 1;
	}

	new->type = type;
	new->actor = context->defender;
	new->card = card;

	context->defender_action = new;

	context->polling = 0;
	MatchAction * a = context->action;
	context->action = a->next;
	freeAction(a);
	return 0;
}

uint8_t postSurrenderAction(MatchContext * context, Relic * relic){
	if(context->polling == 0)
		return 1;

	if(context->action->type != POLL_DEFEND_ACTION)
		return 1;
	
	MatchAction * new = (MatchAction*) calloc(sizeof(MatchAction), 1);
	new->type = SURRENDER_ACTION;
	new->actor = context->defender;
	new->target = context->attacker;
	new->relic = relic;

	context->defender_action = new;

	context->polling = 0;
	MatchAction * a = context->action;
	context->action = a->next;
	freeAction(a);
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

			if ((card == NULL) ^ (card->type != MOVE_EXIT_CARD)) {
				enqueueRollDiceAction(context);
				enqueueMoveRollBonusAction(context, character);
				enqueuePollMoveAction(context, character);
			}
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
		case END_MATCH_ACTION: return "END_MATCH_ACTION";
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
		case ATTACK_DAMAGE_ACTION: return "ATTACK_DAMAGE_ACTION";
		case DEAL_DAMAGE_ACTION: return "DEAL_DAMAGE_ACTION";
		case REST_ACTION: return "REST_ACTION";
		case DEFEND_ACTION: return "DEFEND_ACTION";
		case ESCAPE_ACTION: return "ESCAPE_ACTION";
		case ESCAPE_ATTEMPT_ACTION: return "ESCAPE_ATTEMPT_ACTION";
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
