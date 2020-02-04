#include <stdio.h>
#include "ai.h"
#include "string.h"

#include "hunter.h"

BotAction * makeBotAction(BotAction * action, char * action_name, Hunter * hunter, enum MatchActionType type) {
	if (action == NULL)
		action = calloc(sizeof(BotAction), 1);

	action->value = 1;
	action->actor = hunter;
	action->type = action_name;
	
	MatchAction * match_action = (MatchAction*) calloc(sizeof(MatchAction), 1);

	match_action->type = type;
	match_action->actor = hunter;

	action->action = match_action;

	return action;
}

BotAction * botActionMax(BotAction * a, BotAction * b) {
	/*
		Returns the action with the greater score.  If one is NULL,
		the other is returned, or NULL if both.  If they are equal
		values, a random one is returned.
	*/
	
	if (b == NULL)
		return a;

	if (a == NULL)
		return a;

	if (a->value == b->value)
		return (rand() % 2) == 0 ? a : b;

	if (b->value > a->value)
		return b;

	return a;
}

void botClearMoveActions(Bot * bot) {
	/*
	   Frees all BotActions the bot has allocated, and removes references to
	   them from the bot.
	*/

	// Function to map onto arrays of move actions, to reduce redundancy
	void clear(BotAction ** actions, int length) {
		for (int n=0; n < length; n++) {
			BotAction * action = actions[n];
			
			if (action == NULL)
				continue;

			free(action);
			actions[n] = NULL;
		}
	}

	clear((BotAction **) &bot->wander_action, 6);
	clear((BotAction **) &bot->exit_action, 6);
	clear((BotAction **) &bot->crate_target_action, 6);
	clear((BotAction **) &bot->move_to_character_action, 6 * PLAYERS_LENGTH);
}

void botTurnAction(Bot * bot, MatchContext * context, Hunter * hunter) {
	// TODO: Always use highest available move card
	Statset * stats = hunterStats(hunter);

	// Healing threshold: heal if below a certain HP
	// This overrides descisions over other actions
	if (stats->hp <= stats->max_hp * bot->priorities.heal_threshold / 100) {
		postTurnAction(context, REST_ACTION, NULL, NULL);
		return;
	}
	
	// Generate movement actions for each possible die roll
	// TODO: do not generate actions where a higher score than the highest hitherto generated and valued is impossible to assign
	botClearMoveActions(bot);
	botGenerateWander(bot, context, hunter);
	botGenerateMoveToExit(bot, context, hunter);
	botGenerateCrateAction(bot, context, hunter);
	botGenerateCombatActions(bot, context, hunter);

	// Score the various categories of move actions

	BotPriorities * priorities = &bot->priorities;

	// Score wander actions
	for (int n=0; n < 6; n++) {
		BotAction * action = bot->wander_action[n];
		
		if (action == NULL)
			continue;

		action->value = priorities->wander;
	}

	// Score move-to-exit actions
	for (int n=0; n < 6; n++) {
		BotAction * action = bot->exit_action[n];

		if (action == NULL)
			continue;

		int value = priorities->exit;

		// TODO: reduce score depending on how many turns it takes to reach the exit

		if (hunter == getHunterWithTarget(context))
			value = priorities->exit_has_target;

		action->value = value;
	}

	// Score move-to-crate actions
	for (int n=0; n < 6; n++) {
		BotAction * action = bot->crate_target_action[n];

		if (action == NULL)
			continue;

		int value = priorities->crate_target_unfound;
		action->value = value;
	}

	// TODO: Score move-to-hunter actions

	// Get highest-value action for each move roll
	for (int n=0; n < 6; n++) {
		BotAction * action = bot->wander_action[n];
		action = botActionMax(action, bot->exit_action[n]);
		action = botActionMax(action, bot->crate_target_action[n]);
		// action = botActionMax(action, bot->move_to_character_action[0][n]);
		bot->move_actions[n] = action;
	}

	// Average all move roll action scores, and use that as the final score for performing a move action
	int move_actions_score = 0;
	for (int n=0; n<6; n++)
		move_actions_score += bot->move_actions[n]->value;
	move_actions_score /= 6;

	// TODO: generate combat actions
	// TODO: generate non-threshold heal action
	// TODO: blocked-in rest action

	// Move action
	// Always use the highest move card
	postTurnAction(context, MOVE_ACTION, NULL, hunterHighestMoveCard(hunter));
}

void botGenerateWander(Bot * bot, MatchContext * context, Hunter * hunter) {
	/*
		Generate one wander action for each possible die roll and card used.

		To calculate the target for a wander action, randomly select a path
		amongst those who's traversed distance from the hunter is greatest
		amongst available paths, while still being the shortest path to that
		point.
	*/

	Statset * stats = hunterStats(hunter);

	for (int roll=0; roll < 6; roll++) {
		// Prepare map path register data
		// TODO: copy data, instead of clearing and pathfinding the map data each time
		mapClearPathList(context);

		PathNode * paths = generatePathsWithin(
				context, hunter->x, hunter->y,
				stats->mov + roll + 1
			);

		// If no paths exist for one roll, they will not exist for any, break
		if (paths == NULL) {
			break;
		}

		// The number of paths is counted to randomly select an available path
		int paths_length = 1;

		paths_length = 1;

		// Filter paths which do not have the greatest distance
		for (PathNode * path = paths; path != NULL; path = path->next_path) {
			PathNode * previous = path->prev_path;
			if (previous == NULL)
				continue;

			// Keep paths of equal distance
			if (path->distance == previous->distance) {
				paths_length++;
				continue;
			}
			
			// If this path's distance is the highest yet, remove previous paths

			if (previous->distance < path->distance) {
				// Point the paths variable to the start of the list
				if (previous == paths)
					paths = path;
				
				paths_length = 1;
				path->prev_path = NULL;
			}
			else {
				// This path's distance is lower, remove it from the list
				previous->next_path = path->next_path;
			}
		}

		// Now that only the longest paths from the hunter are found in
		// `paths`, randomly select one of them.
		PathNode * wander_path = paths;
		for (int n=rand() % paths_length; n; n--)
			wander_path = wander_path->next_path;

		// Store this wander action

		BotAction * wander_action = makeBotAction(NULL, "move.wander", hunter, MOVE_ACTION);
		wander_action->action->x = wander_path->x;
		wander_action->action->y = wander_path->y;

		bot->wander_action[roll] = wander_action;
	}
}
	
void generateTargetedMoveActions(Bot * bot, MatchContext * context, Hunter * hunter, int x, int y, char * action_name, BotAction * (*action_array)[6]) {
	mapClearPathList(context);
	PathNode * path = findPath(context, hunter->x, hunter->y, x, y);

	Statset * stats = hunterStats(hunter);
	
	// If no path exists, replace the move actions
	// with NULL, then exit.
	if(path == NULL) {
		for (int n=0; n < 6; n++)
			*action_array[n] = NULL;
		return;
	}

	Card * move_card = hunterHighestMoveCard(hunter);
	int move_bonus = 0;
	if (move_card)
		move_bonus = move_card->num;

	for (int roll=1; roll <= 6; roll++) {
		int max_distance = stats->mov + roll + move_bonus;

		// Find maximum traversable distance along this path
		PathNode * endpoint = path;
		for (; endpoint->to ; endpoint = endpoint->to) {
			if (endpoint->distance >= max_distance)
				break;
		}

		// Store action

		BotAction * move_action = makeBotAction(NULL, action_name, hunter, MOVE_ACTION);
		
		move_action->action->x = endpoint->x;
		move_action->action->y = endpoint->y;
		move_action->action->card = move_card;
		(*action_array)[roll-1] = move_action;
	}

	/*
	for (int roll=0; roll < 6; roll++) {
		if (*action_array[roll] == NULL) {
			printf("Bot warning: did not make all actions for %s\n", action_name);
			break;
		}
	}
	*/
}

void botGenerateMoveToExit(Bot * bot, MatchContext * context, Hunter * hunter) {
	generateTargetedMoveActions(
				bot, context, hunter,
				context->exit_x, context->exit_y,
				"move.exit",
				&bot->exit_action
			);
}

void botGenerateCrateAction(Bot * bot, MatchContext * context, Hunter * hunter) {
	mapClearPathList(context);

	generatePathsWithin(
			context, hunter->x, hunter->y,
			(hunter->stats.mov + 9) * 3  // Only look within three turns of maximum movement
		);

	// Find the crate with the closest path
	Crate * crate = NULL;
	int crate_distance = 0;

	for (int n=0; n < context->crates_len; n++) {
		Crate * new_crate = &context->crates[n];

		// Skip crates which don't exist
		if (!new_crate)
			continue;

		if (!new_crate->exists)
			continue;
		
		// Get path to crate
		PathNode * path = &getTile(context, new_crate->x, new_crate->y)->path;

		// Skip crates to which no path exists
		if (!path->scanned)
			continue;

		// Just go with this crate if it's the first one, then continue.
		if (crate == NULL) {
			crate = new_crate;
			crate_distance = path->distance;
			continue;
		}
		
		// Skip further crates
		if (path->distance > crate_distance)
			continue;
		else if (path->distance == crate_distance) {
			// For two crates of equal distance, 50/50 chance of changing crates
			if (rand() % 2)
				continue;
		}

		// Go with this crate!
		crate_distance = path->distance;
		crate = new_crate;
	}

	// If there is no path to a crate, zero out the crate actions and exit
	if (crate == NULL) {
		for (int n = 0; n < 6; n++)
			bot->crate_target_action[n] = NULL;
		return;
	}

	// Now that a crate has been chosen, generate an action towards it
	generateTargetedMoveActions(
			bot, context, hunter,
			crate->x, crate->y,
			"move.crate",
			&bot->crate_target_action
		);
}

void botMoveAction(Bot * bot, MatchContext * context, Hunter * hunter) {
	// Pick highest-scoring cached move action for this roll and use that
	int roll = context->dice[0]-1;
	BotAction * action = bot->move_actions[roll];

	// printf("Bot action: %s (%d, %d) -> (%d, %d)\n", action->type, hunter->x, hunter->y, action->action->x, action->action->y);
	
	postMoveAction(
			context, hunter,
			action->action->x,
			action->action->y
		);
}

void botAttackAction(Bot * bot, MatchContext * context, Hunter * hunter) {
	postAttackerCard(context, NULL);
}

void botCombatCardAction(Bot * bot, MatchContext * context, Hunter * hunter) {}

void botDefendAction(Bot * bot, MatchContext * context, Hunter * hunter) {
	/*
	   When a bot is attacked, it is prompted whether to
	   counterattack, defend, escape, or surrender an item.  The bot
	   will weigh each of these outcomes in accordance with their
	   priorities, and post the highest-scoring action.
	*/

	Hunter * attacker = context->attacker;

	CombatResultsSpread attacker_unscathed;
	CombatResultsSpread attacker_countered;
	CombatResultsSpread defender_counters;
	CombatResultsSpread defender_blocks;
	// CombatResultsSpread defender_escapes;

	counterattackProbability(attacker, hunter, &attacker_countered, &defender_counters);
	defendProbability(attacker, hunter, &defender_blocks);
	// escapeProbability(attacker, hunter, &defender_escapes);

	BotAction counter_action;
	BotAction defend_action;
	//BotAction escape_action;

	makeBotAction(&counter_action, "counter", hunter, ATTACK_ACTION);
	makeBotAction(&defend_action, "defend", hunter, DEFEND_ACTION);

	BotAction * action = &counter_action;

	postDefenderAction(context, action->action->type, NULL);
}

void botCombatAction(Bot * bot, MatchContext * context, Hunter * hunter) {
	BotAction * max_action;
	Hunter * final_target = NULL;

	for (int n=0; n < PLAYERS_LENGTH; n++) {
		BotAction * action = bot->combat_action[n];

		if (action == NULL)
			continue;

		Hunter * target = action->action->target;
		
		if ( // Hunters are adjacent (non-diagonally)
			((hunter->x == target->x) && (abs(hunter->y - target->y) == 1)) ^
			((hunter->y == target->y) && (abs(hunter->x - target->x) == 1))
		){
			if (max_action == NULL)
				max_action = action;
			else if (action->value > max_action->value)
				max_action = action;
		}
	}

	if (max_action)
		final_target = max_action->action->target;
	
	postCombatAction(context, hunter, final_target);
}

void botGenerateCombatActions(Bot * bot, MatchContext * context, Hunter * hunter) {
	for (int n=0; n < PLAYERS_LENGTH; n++) {
		if (bot->combat_action[n] != NULL) {
			free(bot->combat_action[n]);
			bot->combat_action[n] = NULL;
		}

		Hunter * target_hunter = context->characters[n];

		if (target_hunter == NULL)
			continue;
		
		CombatResultsSpread attacker_spread;
		CombatResultsSpread defender_spread;

		counterattackProbability(hunter, target_hunter, &attacker_spread, &defender_spread);
		
		int score;
		score  = combatSpreadDamageScore(&attacker_spread, bot->priorities.take_damage);
		score += combatSpreadDamageScore(&defender_spread, bot->priorities.deal_damage);
		score += defender_spread.hp_spread[0] * bot->priorities.kill / defender_spread.total_outcomes;
		score += attacker_spread.hp_spread[0] * bot->priorities.die  / attacker_spread.total_outcomes;

		BotAction * combat_action = makeBotAction(NULL, "combat", hunter, COMBAT_ACTION);
		combat_action->value = score;
		combat_action->action->target = target_hunter;

		bot->combat_action[n] = combat_action;
	}
}

void botControllerHook(MatchContext * context, Hunter * hunter, void * controller_data) {
	Bot * bot = (Bot *) controller_data;

	// Don't make an action if the game doesn't want one
	if (!context->polling)
		return;

	// Generate goals based on the context's current action state

	switch (context->action->type) {
		// Decide an action type
		case POLL_TURN_ACTION:
			botTurnAction(bot, context, hunter);
			break;

		case POLL_MOVE_CARD_ACTION:
			// TODO: deprecate POLL_MOVE_CARD_ACTION
			break;

		case POLL_MOVE_ACTION:
			botMoveAction(bot, context, hunter);

		case POLL_ATTACK_ACTION:
			botAttackAction(bot, context, hunter);
			break;

		case POLL_COMBAT_CARD_ACTION:
			botCombatCardAction(bot, context, hunter);
			break;

		case POLL_COMBAT_ACTION:
			botCombatAction(bot, context, hunter);
			break;

		case POLL_DEFEND_ACTION:
			botDefendAction(bot, context, hunter);
			break;

		default:
			break;
	}
}


void combatSpreadMultiply(CombatResultsSpread * spread, int numerator, int denominator) {
	/*
	   Fractional combat probability spread multiplication
	*/

	spread->total_outcomes *= denominator;
	for (int hp = 0; hp <= spread->max_hp; hp++)
		spread->hp_spread[hp] *= numerator;
}


int combatSpreadDamageScore(CombatResultsSpread * spread, int points){
	/*
	   Given `points` per damage, return how many points the combat spread
	   returns, based on a weighted average of damage dealt.
	*/

	int64_t damage_total = 0;
	for (int hp = 0; hp <= spread->max_hp; hp++)
		damage_total += (spread->max_hp - hp) * points * spread->hp_spread[hp];

	return damage_total / spread->total_outcomes;
}

int rollInstances(int total) {
	/*
	   Returns the number of number of 2d6 permutations who's sum
	   is equal to the specified total, which when divided by the
	   total number of permutations results in the probability of
	   that total.
	*/

	int instances = total - 1;
	if (instances > 7)
		instances -= (total - 7) * 2;
	return instances;
}

int simulateAttack(Hunter * attacker, Hunter * defender, int permutation) {
	/*
	   Returns the HP of the recepiant of a simulated attack,
	   given the permutation number of their combination of die
	   rolls.
	*/

	Statset * attacker_stats = &attacker->stats;
	Statset * defender_stats = &defender->stats;

	int attacker_roll = permutation / 11 + 2;
	int defender_roll = permutation % 11 + 2;

	int defender_hp = defender_stats->hp;

	int damage = attacker_roll + attacker_stats->atk - defender_roll - defender_stats->def;
	if (damage < 0)
		damage = 0;

	defender_hp -= damage;
	if (defender_hp < 0)
		return 0;

	return defender_hp;
}

void defendProbability(Hunter * attacker, Hunter * defender, CombatResultsSpread * defender_spread) {
	/*
	   Calculates the probability of each possible HP total which would result
	   from one hunter choosing to block against another's attack.
	*/

	memset(defender_spread, 0, sizeof(CombatResultsSpread));
	defender_spread->hunter = defender;
	Statset * attacker_stats = hunterStats(attacker);
	Statset * defender_stats = hunterStats(defender);

	// Defend actions double base defence.  Temporarily raise stat,
	// for the purpose of simulation.
	defender_stats->def += defender->base_stats.def / 2;

	for (int permutation = 0; permutation < 121; permutation++) {
		int attacker_roll = permutation % 11 + 2;
		int defender_roll = permutation % 11 + 2;

		int instances = rollInstances(attacker_roll) * rollInstances(defender_roll);
		int defender_hp = simulateAttack(attacker, defender, permutation);
		
		// Store probalistic data

		defender_spread->total_outcomes = instances;
		defender_spread->hp_spread[defender_hp] += instances;

		if (defender_spread->max_hp < defender_hp)
			defender_spread->max_hp = defender_hp;
	}
}

void counterattackProbability(Hunter * attacker, Hunter * defender, CombatResultsSpread * attacker_spread, CombatResultsSpread * defender_spread) {
	/*
		Calculates the probability of each possible HP total which could result from an attack and counterattack between two hunters. 

		TODO: calculate based on cards in hand
	*/

	// Resest results spreads
	memset(attacker_spread, 0, sizeof(CombatResultsSpread));
	memset(defender_spread, 0, sizeof(CombatResultsSpread));

	attacker_spread->hunter = attacker;
	defender_spread->hunter = defender;
	
	Statset * attacker_stats = hunterStats(attacker);
	Statset * defender_stats = hunterStats(defender);

	/*
	   Iterate through every combination of dice roll totals between the two players.
	*/
	
	for (int permutation = 0; permutation < 121; permutation++) {
		int attacker_roll = permutation / 11 + 2;
		int defender_roll = permutation % 11 + 2;

		uint32_t instances = rollInstances(attacker_roll) * rollInstances(defender_roll);

		int defender_hp = simulateAttack(attacker, defender, permutation);

		// Store probalistic data

		defender_spread->total_outcomes += instances;
		defender_spread->hp_spread[defender_hp] += instances;

		if (defender_spread->max_hp < defender_hp)
			defender_spread->max_hp = defender_hp;
	}

	/*
		Calculate initial attacker's HP after counterattack from
		defender.
	*/

	int kill_instances = defender_spread->hp_spread[0];

	if (kill_instances == defender_spread->total_outcomes) {
		// In the event of a 100% kill chance of the defender, the attacker's HP has no chance of change
		int hp = attacker_stats->hp;
		attacker_spread->hp_spread[hp] = 1;
		attacker_spread->max_hp = hp;
		attacker_spread->total_outcomes = 1;
	}
	else {

		// Calculate attacker HP spread from defender counterattack

		for (int permutation = 0; permutation < 121; permutation++) {
			int attacker_roll = permutation / 11 + 2;
			int defender_roll = permutation % 11 + 2;

			uint32_t instances = rollInstances(attacker_roll) * rollInstances(defender_roll);

			int attacker_hp = simulateAttack(defender, attacker, permutation);

			// Store probalistic data

			attacker_spread->total_outcomes += instances;
			attacker_spread->hp_spread[attacker_hp] += instances;

			if (attacker_spread->max_hp < attacker_hp)
				attacker_spread->max_hp = attacker_hp;
		}

		// Adjust probability of attacker's HP totals by the chance of the
		// attacker killing the defender before they counter

		if (kill_instances) {
			int old_total = attacker_spread->total_outcomes;
			attacker_spread->total_outcomes = old_total * defender_spread->total_outcomes / (defender_spread->total_outcomes - kill_instances);
			attacker_spread->hp_spread[attacker_stats->hp] += attacker_spread->total_outcomes - old_total;
		}
	}
}

int combatTest() {
	Hunter hunters[] = {
		{	.name = "Daniel", .level = 1, .type = "hunter",
			.base_stats={.atk = 4, .mov = 1, .def = 4, .max_hp=1}
		},
		{	.name = "Dave", .level = 1, .type = "hunter",
			.base_stats = {.atk = 8, .mov=1, .def = 1, .max_hp=1}
		},
		{	.name = "Stan", .level = 1, .type = "hunter",
			.base_stats = {.atk = 2, .mov = 3, .def = 4, .max_hp=1}
		},
		{	.name = "Tim", .level = 1, .type = "hunter",
			.base_stats = {.atk = 30, .mov=3, .def = 2, .max_hp=2}
		}
	};

	void try(int damage, int harm, int kill, int die) {
		for (int n = 0; n < 16; n++) {
			Hunter * attacker = &hunters[n/4];
			Hunter * defender = &hunters[n%4];

			attacker->base_stats.hp = hunterStats(attacker)->max_hp;
			defender->base_stats.hp = hunterStats(defender)->max_hp;

			if (attacker == defender)
				continue;
		
			CombatResultsSpread attacker_spread;
			CombatResultsSpread defender_spread;
			counterattackProbability(attacker, defender, &attacker_spread, &defender_spread);

			int score;
			score  = combatSpreadDamageScore(&attacker_spread, harm);
			score += combatSpreadDamageScore(&defender_spread, damage);
			score += defender_spread.hp_spread[0] * kill / defender_spread.total_outcomes;
			score += attacker_spread.hp_spread[0] * die  / attacker_spread.total_outcomes;

			printHunter(attacker);
			printf(" -> ");
			printHunter(defender);
			printf(" : %d", score);
			printf("\n");
		}
	}

	try(100, -100, 500, -1000);
	printf("\n");
	try(100, -120, 0, 0);

	return 0;
}

int botMain() {
	Hunter hunters[] = {
		{	.name = "Daniel", .level = 1, .type = "hunter",
			.base_stats={.atk = 7, .mov = 4, .def = 3, .max_hp=1}
		},
		{	.name = "Dave", .level = 1, .type = "hunter",
			.base_stats = {.mov = 1, .atk = 11, .def = 1, .max_hp=1}
		},
		{	.name = "Stan", .level = 1, .type = "hunter",
			.base_stats = {.atk = 2, .mov = 3, .def = 8, .max_hp=1}
		},
		{	.name = "Tim", .level = 1, .type = "hunter",
			.base_stats = {.atk = 11, .mov = 1, .def = 1, .max_hp=1}
		}
	};

	Bot * bot = calloc(sizeof(Bot), 1);

	// Assign a priority set which technically
	// makes it possible for bots to win.
	bot->priorities.crate_target_unfound = 3;
	bot->priorities.exit_has_target = 3;
	bot->priorities.wander = 2;
	bot->priorities.exit = 4;

	// Assign this bot to all hunters
	for (int n=0; n < 4; n++) {
		Hunter * hunter = &hunters[n];
		hunter->controller_data = bot;
		hunter->controller_hook = botControllerHook;
	}

	Relic floppy = {.item_id=0, .name="floppy"};
	Relic metal = {.item_id=3, .name="metal"};

	Crate * crates = (Crate*) calloc(sizeof(Crate), 2);
	crates[0].exists = 1;
	crates[0].contents = &floppy;

	crates[1].exists = 1;
	crates[1].contents = &metal;

	MatchContext context = {
		.characters = {
			&hunters[0], &hunters[1],
			&hunters[2], &hunters[3],
		},
		.crates_len = 2,
		.crates = crates,
		.target_relic = &floppy,
		.scoring_context = &DEFAULT_SCORING_CONTEXT
	};

	decodeMap(&context,
			"   #C### ##\n"
			"# #H########\n"
			"#####     ##\n"
			" ##       ##\n"
			" ####     ##\n"
			"  #E#    #H#####\n"
			" ###     ###  ##\n"
			" ##H      ######\n"
			" ### ### ###\n"
			" #####C#H###\n"
			"  ### # ###\n"
		);

	initMatch(&context);

	while (		(context.action != NULL) &&
				(context.action->type != END_MATCH_ACTION)
			)
		matchCycle(&context);

	return 0;
}
