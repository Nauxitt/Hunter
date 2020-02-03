#ifndef __AI_H
#define __AI_H

#include "hunter.h"

typedef struct _CombatResultsSpread {
	Hunter * hunter;
	int total_outcomes;
	int max_hp;
	int total_damage;
	int hp_spread[100];
} CombatResultsSpread;

typedef struct _BotAction {
	// Value, in the context of choosing one goal over another.  
	// Values default to 1, and then are multiplied for weighting,
	// If the action is not to be taken, it's value is set to zero.
	int value;

	char * type;
	
	// Optionally specify how many turns an action will take, which may modify
	// how an action is weighted.
	int turns;
	
	Hunter * actor;
	MatchAction * action;
} BotAction;

typedef struct _BotPriorities {
	/*
	   Percentage of HP, denoting when to override
	   other actions and heal.
	*/
	uint8_t heal_threshold;
	
	/*
	   Scoring for move actions
	*/
	int exit;
	int exit_has_target;
	int wander;
	int crate_target_found;
	int crate_target_unfound;
	
	/*
	   Offensive scoring
	*/
	int kill;
	int kill_target;
	int deal_damage;
	int deal_damage_target;
	
	/*
	   Negative combat result scoring
	*/
	int die;
	int die_with_target;
	int take_damage;
	int take_damage_with_target;
	int surrender;
	int surrender_target;
} BotPriorities;

typedef struct _Bot {
	BotPriorities priorities;
	MatchContext * context;

	// For storing different profiles of move actions
	BotAction * wander_action[6];
	BotAction * exit_action[6];
	BotAction * crate_target_action[6];
	BotAction * move_to_character_action[PLAYERS_LENGTH][6];

	// For storing final move actions
	BotAction * move_actions[6];     // one for each move roll

	// For storing combat actions
	BotAction * combat_action[PLAYERS_LENGTH];
	BotAction * combat_response_action[PLAYERS_LENGTH];

	BotAction * counterattack_action[PLAYERS_LENGTH];
	BotAction * defend_action[PLAYERS_LENGTH];
	BotAction * surrender_action[PLAYERS_LENGTH][INVENTORY_LIMIT];
	BotAction * escape_action[PLAYERS_LENGTH];

} Bot;

void generateTargetedMoveActions(Bot * bot, MatchContext * context, Hunter * hunter, int x, int y, char * action_name, BotAction * (*action_array)[6]);

BotAction * botActionMax(BotAction * a, BotAction * b);
void botGenerateMoveToExit(Bot * bot, MatchContext * match, Hunter * hunter);
void botGenerateWander(Bot * bot, MatchContext * match, Hunter * hunter);
void botGenerateCrateAction(Bot * bot, MatchContext * match, Hunter * hunter);
void botGenerateCombatActions(Bot * bot, MatchContext * context, Hunter * hunter);

BotAction * makeBotAction(BotAction * action, char * action_name, Hunter * hunter, enum MatchActionType type);
void botClearMoveActions(Bot * bot);

void botControllerHook(MatchContext * context, Hunter * hunter, void * controller_data);
void botTurnAction(Bot * bot, MatchContext * match, Hunter * hunter);
void botMoveAction(Bot * bot, MatchContext * match, Hunter * hunter);
void botAttackAction(Bot * bot, MatchContext * match, Hunter * hunter);
void botCombatCardAction(Bot * bot, MatchContext * match, Hunter * hunter);
void botCombatAction(Bot * bot, MatchContext * match, Hunter * hunter);
void botDefendAction(Bot * bot, MatchContext * match, Hunter * hunter);

void combatSpreadMultiply(CombatResultsSpread * spread, int numerator, int denominator);
void combatProbability(Hunter * attacker, Hunter * defender, CombatResultsSpread * attacker_spread, CombatResultsSpread * defender_spread);
int combatSpreadDamageScore(CombatResultsSpread * spread, int points);

int botMain();

#endif
