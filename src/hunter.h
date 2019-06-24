/*
   Specifies datatypes for the gameplay's RPG features, primarily consisting of hunters and their stats.
*/

#ifndef __hunter_h
#define __hunter_h

#include <stdint.h>

typedef struct _Statset Statset;
typedef struct _Hunter Hunter;
typedef struct _Item Item;
typedef struct _Card Card;

typedef struct _Statset {
	uint8_t atk;
	uint8_t mov;
	uint8_t def;
	uint8_t hp;
	uint8_t max_hp;

	uint8_t escape_chance;
	uint8_t evade_trap_chance;
} Statset;

typedef struct _Item {
	char name[7];
	Statset (*statModifier)(Hunter * hunter);
} Item;

enum CardType {
	NULL_CARD,

	MOVE_CARD,           // Ranged 1-3, increases MOV
	MOVE_EXIT_CARD,      // Teleports to EXIT or escapes combat

	ATTACK_CARD,         // Ranged 1-9, increases ATK
	ATTACK_DOUBLE_CARD,  // Doubles your ATK power
	ATTACK_COPY_CARD,    // Adds your opponent's ATK to yours

	EMPTY_TRAP_CARD,     // Empties hunter's hand
	STUN_TRAP_CARD,      // Skips hunter's next turn
	DAMADE_TRAP_CARD,    // Damages hunter
	LEG_DAMAGE_CARD,     // Reduces hunter's MOV to zero

	DEFENSE_CARD,        // Ranged 1-9
	DEFENSE_ALL_CARD,    // Prevents all damage or evades all traps
	DEFENSE_DOUBLE_CARD  // Doubles defense or evades all traps
};

typedef struct _Card {
	enum CardType type;
	int num;
} Card;

#define NAME_MAX_LENGTH 8
#define HAND_LIMIT 6
#define INVENTORY_LIMIT 6

typedef struct _Hunter {
	Statset stats;
	Statset turn_stats;
	Statset base_stats;
	char name[NAME_MAX_LENGTH + 1];
	Item * inventory[INVENTORY_LIMIT];
	Card * hand[HAND_LIMIT];
	int credits;
} Hunter;

enum BoardActionType {
	MOVE_ACTION,
	ATTACK_ACTION,
	REST_ACTION
};

enum CobatActionType {
	COUNTERATTACK_ACTION,
	DEFEND_ACTION,
	ESCAPE_ACTION,
	SURRENDER_ACTION
};

typedef struct _Battle {
	Hunter * hunters[4];
	Item * target_item;
	// Map *
	// Flag * flags;
	// Monster * monsters;
	// Monster * gon;
	// Card cards[100];
	// int top_card;
	// Crate * crates[n];
} Battle;

Statset * hunterStats(Hunter * h);
Card * hunterPopCard(Hunter * h, int card_num);
int hunterHandSize(Hunter * h);

#endif
