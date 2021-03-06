/*
   combatstate.h

   GameState which acts as an interface to a MatchContext's combat actions.
*/

#ifndef __combatstate_h
#define __combatstate_h

#include "stateengine.h"
#include "mapstate.h"
#include "menubar.h"

typedef struct _CombatState {
	GameState state;

	MatchContext * match;
	MenubarState * menubar;
	HunterEntity attacker_entity;
	HunterEntity defender_entity;

	int selector;
	Card * card_selected;

	Relic * relic_selected;
} CombatState;

#define CombatState(ptr) ((CombatState*) ptr)

CombatState * makeCombatState(CombatState * state, MatchContext * match);
void combatOnEnter(EventHandler * h);
void combatOnExit(EventHandler * h);
void combatOnTick(EventHandler * h);
void combatOnDraw(EventHandler * h);
void combatOnKeyUp(EventHandler * h, SDL_Event * e);

#endif
