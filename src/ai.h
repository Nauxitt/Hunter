#ifndef __AI_H
#define __AI_H

#include "hunter.h"

typedef struct _CombatResultsSpread {
	Hunter * hunter;
	float hp_probability[100];
} CombatResultsSpread;

typedef struct _BotAction {
	// Value, in the context of choosing one goal over another.  
	// Values default to 1, and then are multiplied for weighting,
	// If the action is not to be taken, it's value is set to zero.
	int value;
	
	// Optionally specify how many turns an action will take, which may modify
	// how an action is weighted.
	int turns;
	
	Hunter * actor;
	MatchAction * action;
} BotAction;

typedef struct _BotPriorities {
	uint8_t heal_threshold;
	
	int exit;
	int exit_has_target;
	int wander;
	int crate_target_found;
	int crate_target_unfound;
	int flag;
	
	int heal_hp;     // per HP
	int draw_card;   // per card
	
	int kill_hunter;
	int kill_target_hunter;

	int damage_dealt;          // per HP
	int damage_taken;          // per HP
	int damage_target_hunter;  // per HP
} BotPriorities;

typedef struct _Bot {
	BotPriorities priorities;
	Hunter * hunter;
	MatchContext * context;

	// For storing different profiles of move actions
	BotAction * wander_action[6];
	BotAction * exit_action[6];
	BotAction * crate_target_action[6];
	BotAction * move_to_character_action[PLAYERS_LENGTH][6];

	// For storing final move actions
	BotAction * move_actions[6];     // one for each move roll
} Bot;

void generateTargetedMoveActions(Bot * bot, MatchContext * match, Hunter * hunter, int x, int y, BotAction * (*action_array)[6]);

BotAction * botActionMax(BotAction * a, BotAction * b);
void botGenerateMoveToExit(Bot * bot, MatchContext * match, Hunter * hunter);
void botGenerateWander(Bot * bot, MatchContext * match, Hunter * hunter);
void botGenerateCrateAction(Bot * bot, MatchContext * match, Hunter * hunter);

BotAction * makeBotAction(BotAction * action, Hunter * hunter, enum MatchActionType type);
void botClearMoveActions(Bot * bot);

void botAction(Bot * bot, MatchContext * match, Hunter * hunter);
void botTurnAction(Bot * bot, MatchContext * match, Hunter * hunter);
void botMoveAction(Bot * bot, MatchContext * match, Hunter * hunter);
void botAttackAction(Bot * bot, MatchContext * match, Hunter * hunter);
void botCombatCardAction(Bot * bot, MatchContext * match, Hunter * hunter);
void botCombatAction(Bot * bot, MatchContext * match, Hunter * hunter);
void botDefendAction(Bot * bot, MatchContext * match, Hunter * hunter);

int botMain();

#endif
