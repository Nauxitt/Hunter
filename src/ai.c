#include "ai.h"
#include "hunter.h"

BotAction * makeBotAction(BotAction * action, Hunter * hunter, enum MatchActionType type){
	if (action == NULL)
		action = calloc(sizeof(BotAction), 1);

	action->value = 1;
	action->actor = hunter;
	
	MatchAction * match_action = (MatchAction*) calloc(sizeof(MatchAction), 1);

	match_action->type = type;
	match_action->actor = hunter;

	return action;
}

BotAction * botActionMax(BotAction * a, BotAction * b) {
	if (a == b)
		return a;

	if (b == NULL)
		return a;

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

void botTurnAction(Bot * bot, MatchContext * match, Hunter * hunter) {
	// TODO: Always use highest available move card
	Statset * stats = hunterStats(hunter);

	// Healing threshold: heal if below a certain HP
	// This overrides descisions over other actions
	if (stats->hp <= stats->max_hp * bot->priorities.heal_threshold / 100) {
		postTurnAction(match, REST_ACTION, NULL, NULL);
		return;
	}
	
	// Generate movement actions for each possible die roll
	botClearMoveActions(bot);
	botGenerateWander(bot, match, hunter);
	botGenerateMoveToExit(bot, match, hunter);
	botGenerateCrateAction(bot, match, hunter);

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

		value = priorities->exit;
		
		// TODO: reduce score depending on how many turns it takes to reach the exit

		if (hunter == getHunterWithTarget(match))
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

	// TODO:Score move-to-hunter actions

	// Get highest-value action for each move roll
	for (int n=0; n < 6; n++) {
		BotAction * action = bot->wander_action[n];

		action = botActionMax(action, bot->exit_action[n]);
		action = botActionMax(action, bot->crate_target_action[n]);
		action = botActionMax(action, bot->move_to_character_action[0][n]);
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
	postTurnAction(match, MOVE_ACTION, NULL, hunterHighestMoveCard(hunter));
}

void botGenerateWander(Bot * bot, MatchContext * match, Hunter * hunter) {
	/*
		Generate one wander action for each possible die roll and card used.

		To calculate the target for a wander action, randomly select a path
		amongst those who's traversed distance from the hunter is greatest
		amongst available paths, while still being the shortest path to that
		point.
	*/

	for (int roll=0; roll < 6; roll++) {
		// Prepare map path register data
		// TODO: copy data, instead of clearing and pathfinding the map data each time
		mapClearPathList(match);

		Statset * stats = hunterStats(hunter);

		PathNode * paths = generatePathsWithin(
				match, hunter->x, hunter->y,
				stats->mov + roll + 1
			);

		// If no paths exist for one roll, they will not exist for any, break
		if (paths == NULL) {
			break;
		}

		// The number of paths is counted to randomly select an available path
		int paths_length = 1;

		// Filter paths which do not have the greatest distance

		for (PathNode * path = paths; path; path = path->next_path) {
			if (path->prev_path == NULL)
				continue;

			// Keep paths of equal distance
			if (path->distance == path->prev_path->distance) {
				paths_length++;
				continue;
			}
			
			// If this path's distance is the highest yet, remove previous paths

			if (path->prev_path->distance < path->distance) {
				// Point the paths variable to the start of the list
				if (path->prev_path == paths)
					paths = path;
				
				paths_length = 1;
				path->prev_path = NULL;
			}
			else {
				// This path's distance is lower, remove it from the list
				path->prev_path->next_path = path->next_path;
			}
		}

		// Now that only the longest paths from the hunter are found in
		// `paths`, randomly select one of them.
		PathNode * wander_path = paths;
		for (int n=rand() % paths_length; n; n--)
			wander_path = wander_path->next_path;

		// Store this wander action

		BotAction * wander_action = makeBotAction(NULL, hunter, MOVE_ACTION);
		wander_action->action->x = wander_path->x;
		wander_action->action->y = wander_path->y;

		bot->wander_action[roll] = wander_action;
	}
}
	
void generateTargetedMoveActions(Bot * bot, MatchContext * match, Hunter * hunter, int x, int y, BotAction * (*action_array)[6]){
	mapClearPathList(match);

	PathNode * path = findPath(match, hunter->x, hunter->y, match->exit_x, match->exit_y);

	Statset * stats = hunterStats(hunter);

	// If no path exists, replace the move actions with NULL
	if(path == NULL)
		for (int n=0; n < 6; n++)
			*action_array[n] = NULL;

	Card * move_card = hunterHighestMoveCard(hunter);
	int move_bonus = 0;
	if (move_card)
		move_bonus = move_card->num;

	for (int roll=5; roll >= 0; roll--) {
		int max_distance = stats->mov + roll + move_bonus + 1;

		PathNode * endpoint = path;
		while (path->to && path->distance <= max_distance)
			path = path->to;

		// Store action

		BotAction * move_action = makeBotAction(NULL, hunter, MOVE_ACTION);
		move_action->action->x = endpoint->x;
		move_action->action->y = endpoint->y;
		move_action->action->card = move_card;
		*action_array[roll] = move_action;
	}
}

void botGenerateMoveToExit(Bot * bot, MatchContext * match, Hunter * hunter) {
	generateTargetedMoveActions(
				bot, match, hunter,
				match->exit_x, match->exit_y,
				&bot->exit_action
			);
}

void botGenerateCrateAction(Bot * bot, MatchContext * match, Hunter * hunter) {
	mapClearPathList(match);

	generatePathsWithin(
			match, hunter->x, hunter->y,
			(hunter->stats.mov + 9) * 3  // Only look within three turns of maximum movement
		);

	// Find the crate with the closest path
	Crate * crate;
	int crate_distance = 0;

	for (int n=0; n < match->crates_len; n++) {
		Crate * new_crate = &match->crates[n];

		// Skip crates which don't exist
		if (!new_crate)
			continue;

		if (!new_crate->exists)
			continue;
		
		// Get path to crate
		PathNode * path = &getTile(match, crate->x, crate->y)->path;

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
			if (rand() % 2 == 0)
				continue;
		}

		// Go with this crate!
		crate_distance = path->distance;
		crate = new_crate;
	}

	// Now that a crate has been chosen, generate an action towards it
	generateTargetedMoveActions(
			bot, match, hunter,
			crate->x, crate->y,
			&bot->exit_action
		);
}

void botMoveAction(Bot * bot, MatchContext * match, Hunter * hunter) {
	// Pick highest-scoring cached move action for this roll and use that
	int roll = match->dice[0];
	BotAction * action = bot->move_actions[roll];
	
	postMoveAction(
			match, hunter,
			action->action->x,
			action->action->y
		);
}

void botAttackAction(Bot * bot, MatchContext * match, Hunter * hunter) {}
void botCombatCardAction(Bot * bot, MatchContext * match, Hunter * hunter) {}

void botCombatAction(Bot * bot, MatchContext * match, Hunter * hunter) {
	// If an attack action exists within the bot, execute it
}

void botDefendAction(Bot * bot, MatchContext * match, Hunter * hunter) {}

void botAction(Bot * bot, MatchContext * match, Hunter * hunter) {
	// Don't make an action if the game doesn't want one
	if (!match->polling)
		return;

	// Generate goals based on the match's current action state

	switch (match->action->type) {
		// Decide an action type
		case POLL_TURN_ACTION:
			botTurnAction(bot, match, hunter);
			break;

		case POLL_MOVE_CARD_ACTION:
			// TODO: deprecate POLL_MOVE_CARD_ACTION
			break;

		case POLL_MOVE_ACTION:
			botMoveAction(bot, match, hunter);

		case POLL_ATTACK_ACTION:
			botAttackAction(bot, match, hunter);
			break;

		case POLL_COMBAT_CARD_ACTION:
			botCombatCardAction(bot, match, hunter);
			break;

		case POLL_COMBAT_ACTION:
			botCombatAction(bot, match, hunter);
			break;

		case POLL_DEFEND_ACTION:
			botDefendAction(bot, match, hunter);
			break;

		default:
			break;
	}
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
	// matchCycle(&context);

	return 0;
}
