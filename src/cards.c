#include "hunter.h"
#include "cards.h"

Card MOVEMENT_DECK[MOVEMENT_DECK_SIZE] = {
	{.type = MOVE_CARD, .num = 1}, {.type = MOVE_CARD, .num = 1},
	{.type = MOVE_CARD, .num = 1}, {.type = MOVE_CARD, .num = 1},
	{.type = MOVE_CARD, .num = 1}, {.type = MOVE_CARD, .num = 1},
	{.type = MOVE_CARD, .num = 1}, {.type = MOVE_CARD, .num = 1},
	{.type = MOVE_CARD, .num = 1}, {.type = MOVE_CARD, .num = 1},

	{.type = MOVE_CARD, .num = 2}, {.type = MOVE_CARD, .num = 2},
	{.type = MOVE_CARD, .num = 2}, {.type = MOVE_CARD, .num = 2},
	{.type = MOVE_CARD, .num = 2}, {.type = MOVE_CARD, .num = 2},
	{.type = MOVE_CARD, .num = 2}, {.type = MOVE_CARD, .num = 2},
	{.type = MOVE_CARD, .num = 2}, {.type = MOVE_CARD, .num = 2},

	{.type = MOVE_CARD, .num = 3}, {.type = MOVE_CARD, .num = 3},
	{.type = MOVE_CARD, .num = 3}, {.type = MOVE_CARD, .num = 3},
	{.type = MOVE_CARD, .num = 3}, {.type = MOVE_CARD, .num = 3},
	{.type = MOVE_CARD, .num = 3}, {.type = MOVE_CARD, .num = 3},
	{.type = MOVE_CARD, .num = 3}, {.type = MOVE_CARD, .num = 3}
};

Card DEFAULT_DECK[DECK_SIZE] = {
	{.type = MOVE_CARD, .num = 1}, {.type = MOVE_CARD, .num = 1},
	{.type = MOVE_CARD, .num = 1}, {.type = MOVE_CARD, .num = 1},
	{.type = MOVE_CARD, .num = 1}, {.type = MOVE_CARD, .num = 1},
	{.type = MOVE_CARD, .num = 1}, {.type = MOVE_CARD, .num = 1},
	{.type = MOVE_CARD, .num = 1}, {.type = MOVE_CARD, .num = 1},
	{.type = MOVE_CARD, .num = 1}, {.type = MOVE_CARD, .num = 1},
	{.type = MOVE_CARD, .num = 1}, {.type = MOVE_CARD, .num = 1},
	{.type = MOVE_CARD, .num = 1}, {.type = MOVE_CARD, .num = 1},

	{.type = MOVE_CARD, .num = 2}, {.type = MOVE_CARD, .num = 2},
	{.type = MOVE_CARD, .num = 2}, {.type = MOVE_CARD, .num = 2},
	{.type = MOVE_CARD, .num = 2}, {.type = MOVE_CARD, .num = 2},
	{.type = MOVE_CARD, .num = 2}, {.type = MOVE_CARD, .num = 2},

	{.type = MOVE_CARD, .num = 3}, {.type = MOVE_CARD, .num = 3},
	{.type = MOVE_CARD, .num = 3}, {.type = MOVE_CARD, .num = 3},

	{.type = MOVE_EXIT_CARD}, {.type = MOVE_EXIT_CARD},

	{.type = ATTACK_CARD, .num = 3}, {.type = ATTACK_CARD, .num = 3}, 
	{.type = ATTACK_CARD, .num = 3}, {.type = ATTACK_CARD, .num = 3}, 
	{.type = ATTACK_CARD, .num = 3}, {.type = ATTACK_CARD, .num = 3}, 
	{.type = ATTACK_CARD, .num = 3}, 

	{.type = ATTACK_CARD, .num = 4}, {.type = ATTACK_CARD, .num = 4}, 
	{.type = ATTACK_CARD, .num = 4}, {.type = ATTACK_CARD, .num = 4}, 
	{.type = ATTACK_CARD, .num = 4}, {.type = ATTACK_CARD, .num = 4}, 
	{.type = ATTACK_CARD, .num = 4}, 

	{.type = ATTACK_CARD, .num = 5}, {.type = ATTACK_CARD, .num = 5}, 
	{.type = ATTACK_CARD, .num = 5}, {.type = ATTACK_CARD, .num = 5}, 
	{.type = ATTACK_CARD, .num = 5}, 

	{.type = ATTACK_CARD, .num = 6}, {.type = ATTACK_CARD, .num = 6}, 
	{.type = ATTACK_CARD, .num = 6}, {.type = ATTACK_CARD, .num = 6}, 
	{.type = ATTACK_CARD, .num = 6}, 

	{.type = ATTACK_CARD, .num = 7}, {.type = ATTACK_CARD, .num = 7}, 
	{.type = ATTACK_CARD, .num = 7}, 

	{.type = ATTACK_CARD, .num = 8}, {.type = ATTACK_CARD, .num = 8}, 
	{.type = ATTACK_CARD, .num = 8}, 

	{.type = ATTACK_CARD, .num = 9}, 
	{.type = ATTACK_DOUBLE_CARD, .num = 0}, 
	{.type = ATTACK_COPY_CARD, .num = 0}, 

	{.type = DEFENSE_CARD, .num = 3}, {.type = DEFENSE_CARD, .num = 3},

	{.type = DEFENSE_CARD, .num = 4}, {.type = DEFENSE_CARD, .num = 4},

	{.type = DEFENSE_CARD, .num = 5}, {.type = DEFENSE_CARD, .num = 5},
	{.type = DEFENSE_CARD, .num = 5},

	{.type = DEFENSE_CARD, .num = 6}, {.type = DEFENSE_CARD, .num = 6},
	{.type = DEFENSE_CARD, .num = 7}, {.type = DEFENSE_CARD, .num = 7},
	{.type = DEFENSE_CARD, .num = 8}, {.type = DEFENSE_CARD, .num = 8},
	{.type = DEFENSE_CARD, .num = 9}, {.type = DEFENSE_CARD, .num = 9},

	{.type = DEFENSE_ALL_CARD},
	{.type = DEFENSE_DOUBLE_CARD}

	/*
	{.type = EMPTY_TRAP_CARD}, {.type = EMPTY_TRAP_CARD},
	{.type = EMPTY_TRAP_CARD}, {.type = EMPTY_TRAP_CARD},
	{.type = EMPTY_TRAP_CARD},

	{.type = STUN_TRAP_CARD}, {.type = STUN_TRAP_CARD},
	{.type = STUN_TRAP_CARD}, {.type = STUN_TRAP_CARD},
	{.type = STUN_TRAP_CARD},

	{.type = DAMADE_TRAP_CARD}, {.type = DAMADE_TRAP_CARD},
	{.type = DAMADE_TRAP_CARD}, {.type = DAMADE_TRAP_CARD},
	{.type = DAMADE_TRAP_CARD},

	{.type = LEG_DAMAGE_CARD}, {.type = LEG_DAMAGE_CARD},
	{.type = LEG_DAMAGE_CARD}, {.type = LEG_DAMAGE_CARD},
	{.type = LEG_DAMAGE_CARD},
	*/
};

Card no_card = {
	.type = NO_CARD
};
