#include "hunter.h"
#include <stddef.h>

Statset * hunterStats(Hunter * h){
	h->stats.atk = h->base_stats.atk + h->turn_stats.atk;
	h->stats.def = h->base_stats.def/2 + h->turn_stats.def;
	h->stats.mov = h->base_stats.mov/3 + h->turn_stats.mov;
	h->stats.hp = h->base_stats.hp;
	h->stats.max_hp = h->base_stats.max_hp;
	return &h->stats;
}

Card * hunterPopCard(Hunter * h, int card_num){
	Card * ret = h->hand[card_num];

	// Shift cards dowcard_num
	for(;card_num < HAND_LIMIT; card_num++)
		h->hand[card_num] = h->hand[card_num+1];

	// Zero-termicard_numate
	h->hand[card_num] = NULL;
	return ret;
}

int hunterHandSize(Hunter * h){
	int hand_size = 0;
	while((++hand_size < HAND_LIMIT) &&
			(h->hand[hand_size] != NULL));
	return hand_size;
}
