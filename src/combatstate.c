#include "combatstate.h"
#include "hunter.h"

CombatState * makeCombatState(CombatState * state, MatchContext * match){
	if(state == NULL)
		state = (CombatState *) calloc(sizeof(CombatState), 1);

	state->match = match;
	
	EventHandler * e = EventHandler(state);
	e->onEnter = combatOnEnter;
	e->onExit = combatOnExit;
	e->onTick = combatOnTick;
	e->onDraw = combatOnDraw;
	e->onKeyUp = combatOnKeyUp;

	return state;
}

void combatOnEnter(EventHandler * h){
	CombatState * state = CombatState(h);
	
	Entity(state->attacker_entity)->x = 0;
	Entity(state->attacker_entity)->y = 0;
	
	Entity(state->defender_entity)->x = 0;
	Entity(state->defender_entity)->y = 0;
}

void combatOnExit(EventHandler * h){
	CombatState * state = CombatState(h);
	
	Entity(state->attacker_entity)->x = state->attacker_entity->hunter->x;
	Entity(state->attacker_entity)->y = state->attacker_entity->hunter->y;
	
	Entity(state->defender_entity)->x = state->defender_entity->hunter->x;
	Entity(state->defender_entity)->y = state->defender_entity->hunter->y;
}

void combatOnTick(EventHandler * h){
	CombatState * state = CombatState(h);
	MatchContext * match = state->match;

	int breaker = 1;
	while((breaker == 1) && (match->action)){
		MatchAction * action = match->action;

		switch(action->type){
			case POLL_ATTACK_ACTION:
			case POLL_DEFEND_ACTION:
			case POLL_COMBAT_CARD_ACTION:
			case POLL_COMBAT_ACTION:
				breaker = 0;

			case ENTER_COMBAT_ACTION:
			case DAMAGE_ACTION:
			case ROLL_DICE_ACTION:
			case MOVE_ROLL_BONUS_ACTION:
			case CATCH_ROLL_BONUS_ACTION:
			case ESCAPE_ROLL_BONUS_ACTION:
			case ATTACK_ROLL_BONUS_ACTION:
			case DEFENSE_ROLL_BONUS_ACTION:
			case EXECUTE_COMBAT_ACTION:
			case ATTACK_ACTION:
			case DEFEND_ACTION:
			case ESCAPE_ACTION:
			case SURRENDER_ACTION:
			case GIVE_RELIC_ACTION:
			case REMOVE_RELIC_ACTION:
				matchCycle(match);
				break;

			// These actions should not occur during the combat state.  In the future, it can throw an error.  Currently, these and EXIT_COMBAT_ACTION exit CombatState
			case BEGIN_MATCH_ACTION:
			case TURN_START_ACTION:
			case TURN_END_ACTION:
			case DRAW_CARD_ACTION:
			case USE_CARD_ACTION:
			case HEAL_ACTION:
			case POLL_TURN_ACTION:
			case POLL_MOVE_CARD_ACTION:
			case POLL_MOVE_ACTION:
			case MOVE_ACTION:
			case MOVE_STEP_ACTION:
			case END_MOVE_ACTION:
			case TELEPORT_ACTION:
			case TELEPORT_RANDOM_ACTION:
			case COMBAT_ACTION:
			case REST_ACTION:
			case DEATH_CHECK_ACTION:
			case OPEN_CRATE_ACTION:

			case EXIT_COMBAT_ACTION:
				gamePopState();
				onTick(EventHandler(game.state));
				return;
		}
	}
}

void combatOnKeyUp(EventHandler * h, SDL_Event * e){
	// CombatState * state = CombatState(h);
	// MatchContext * match = state->match;
}

void combatOnDraw(EventHandler * h){
	// CombatState * state = CombatState(h);
	// MatchContext * match = state->match;
}
