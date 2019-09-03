/*
   Specifies datatypes for the gameplay's RPG features, primarily consisting of hunters and their stats.  Also contains functions for headless gameplay logic. 
*/

#ifndef __hunter_h
#define __hunter_h

#include <stdint.h>
#include "cards.h"

typedef struct _Statset Statset;
typedef struct _Hunter Hunter;
typedef struct _Relic Relic;
typedef struct _Card Card;

typedef struct _Statset {
	uint8_t hp;
	uint8_t max_hp;
	uint8_t atk;    // Adds to damage in combat
	uint8_t mov;    // Adds to map movement and combat escape attemps
	uint8_t def;    // Subtracts from damage in combat
	
	// Invisible bonuses granted by certain items:
	uint8_t escape_chance;
	uint8_t evade_trap_chance;
} Statset;


typedef struct _Relic {
	char name[7];
	Statset (*statModifier)(Hunter * hunter);
	int item_id;
	int target_item;
} Relic;

#define CHARACTER_TYPE_MAX_LENGTH 8
#define NAME_MAX_LENGTH 8
#define HAND_LIMIT 6
#define INVENTORY_LIMIT 6

typedef struct _Hunter {
	int id;
	Statset stats;
	Statset turn_stats;
	Statset dice_stats;
	Statset base_stats;
	char name[NAME_MAX_LENGTH + 1];
	char type[CHARACTER_TYPE_MAX_LENGTH];
	int level;
	Relic * inventory[INVENTORY_LIMIT+1];
	Card * hand[HAND_LIMIT];
	int x, y;
	int credits;
} Hunter;


typedef struct _Crate {
	Relic * contents;
	int x, y;
	int exists;
} Crate;


typedef struct _Tile {
	int x, y;
	uint8_t exists;
	Hunter * hunter;
	Crate * crate;
	// Flag * flag;
	// Trap * traps[3]
} Tile;


enum MatchActionType {
	BEGIN_MATCH_ACTION,
	TURN_START_ACTION,
	TURN_END_ACTION,
	
	// Turn actions
	DRAW_CARD_ACTION,
	USE_CARD_ACTION,
	HEAL_ACTION,
	DAMAGE_ACTION,
	
	ROLL_DICE_ACTION,
	
	// Actions for applying dice bonuses to stats
	MOVE_ROLL_BONUS_ACTION,
	CATCH_ROLL_BONUS_ACTION,
	ESCAPE_ROLL_BONUS_ACTION,
	ATTACK_ROLL_BONUS_ACTION,
	DEFENSE_ROLL_BONUS_ACTION,
	
	// Polling actions, which lock the state until input is recieved
	POLL_TURN_ACTION,
	POLL_MOVE_CARD_ACTION,
	POLL_MOVE_ACTION,
	POLL_ATTACK_ACTION,
	POLL_DEFEND_ACTION,
	POLL_COMBAT_CARD_ACTION,
	POLL_COMBAT_ACTION,

	// Board actions
	MOVE_ACTION,
	MOVE_STEP_ACTION,
	END_MOVE_ACTION,
	TELEPORT_ACTION,
	TELEPORT_RANDOM_ACTION,
	COMBAT_ACTION,  // invoked in the map to initiate combat with another hunter
	REST_ACTION,
	
	// Combat actions
	ENTER_COMBAT_ACTION,
	EXIT_COMBAT_ACTION,
	EXECUTE_COMBAT_ACTION,
	ATTACK_DAMAGE_ACTION,
	ESCAPE_ATTEMPT_ACTION,
	
	//  User-selected combat actions
	ATTACK_ACTION,
	DEFEND_ACTION,
	ESCAPE_ACTION,
	SURRENDER_ACTION,

	DEATH_CHECK_ACTION,  // Randomly teleport hunter if at zero HP
	OPEN_CRATE_ACTION,
	GIVE_RELIC_ACTION,
	REMOVE_RELIC_ACTION
};

enum ControllerType {
	CONTROLLER_SLEEPBOT,
	CONTROLLER_LOCAL_HUMAN,
	CONTROLLER_NETWORK,
	CONTROLLER_BOT
};

typedef struct _Agent {
	enum ControllerType type;
} Agent;

typedef struct _MatchAction {
	enum MatchActionType type;
	struct _MatchAction * next;
	Hunter * actor;
	Hunter * target;

	Card * card;
	Crate * crate;
	Relic * relic;
	int x, y;
	int value;
} MatchAction;


/*
   Contains data pertaining to one game match, and should allow the simulation of a match, independant of how it may be displayed, or how it may recieve input.
*/
typedef struct _MatchContext {
	MatchAction * action;
	MatchAction * enqueue;  // Used internally for adding actions in FIFO order to the FILO action stack
	
	/*
	   When the context is in polling mode, it will not remove actions from the stack unless an action has been stored in postedAction.  This allows control from outside functions.
	*/
	uint8_t polling;
	MatchAction * postedAction;
	
	/*
		List of game characters,  null-terminated

		NOTE: this number will be increased from four to account for the enemies which can spawn on the map, as well as for Gon.  This will require looking into the turn order.  If the enemies take their turns in between players, since the ordering of this array reflects turn order, space needs to be reserved in between elements denoting player characters, with NULL pointers being skipped over in the case of enemies which haven't spawned yet.
	*/
	Hunter * characters [4 + 1];
	int active_player;
	
	int dice[4];      // The values of each rolled die
	int dice_total;   // The sum of the first two dice
	int dice_total2;   // The sum of the last two dice

	int exit_x, exit_y;

	Relic * target_relic;

	int crates_len;
	Crate * crates;

	Card * deck[DECK_SIZE];
	int deck_len;

	int map_w, map_h;
	Tile * map;

	// Combat-specific properties
	Hunter * attacker;
	Hunter * defender;

	Card * attacker_card;
	Card * defender_card;

	MatchAction * defender_action;
} MatchContext;

Crate * getCrateAt(MatchContext * context, int x, int y);
Hunter * getHunterAt(MatchContext * context, int x, int y);
uint8_t hunterAt(Hunter * hunter, int x, int y);

Statset * hunterStats(Hunter * h);
Card * hunterPopCard(Hunter * h, int card_num);
int hunterHandSize(Hunter * h);
void hunterUseCard(MatchContext * context, Hunter * hunter, Card * card);
int hunterInventoryLength(Hunter * hunter);
int hunterAddRelic(Hunter * hunter, Relic * relic);
Relic * hunterRemoveRelicAt(Hunter * hunter, int index);
int hunterRemoveRelic(Hunter * hunter, Relic * relic);

void initMatch(MatchContext * context);
void matchCycle(MatchContext * context);
void rollDice(MatchContext * context);

uint8_t postTurnAction(MatchContext * context, enum MatchActionType type, Hunter * character, Card * card);
uint8_t postMoveCardAction(MatchContext * context, Hunter * character, Card * card);
uint8_t postMoveAction(MatchContext * context, Hunter * character, int x, int y);
uint8_t postDefenderAction(MatchContext * context, enum MatchActionType type, Card * card);
uint8_t postSurrenderAction(MatchContext * context, Relic * relic);

uint8_t postCombatAction(MatchContext * context, Hunter * attacker, Hunter * defender);
uint8_t postAttackerCard(MatchContext * context, Card * card);
void printMatchQueue(MatchContext * context);
void printMatchAction(MatchAction * action);
void matchQueueUpdate(MatchContext * context);
int matchQueueLength(MatchContext * context);
const char * getMatchActionName(enum MatchActionType type);

void getRandomTile(MatchContext * context, int * x, int * y);
#endif
