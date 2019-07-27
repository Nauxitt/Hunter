#ifndef __cards_h
#define __cards_h

#include "hunter.h"

#define DECK_SIZE 30

enum CardType {
	NO_CARD,             // Used to denote that no card is selected for the given action.
	UNKNOWN_CARD,        // Reserved for networked games

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

extern Card CARDS[DECK_SIZE];

#endif
