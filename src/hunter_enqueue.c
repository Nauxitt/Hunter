#include <stdlib.h>
#include "hunter.h"

MatchAction * matchEnqueueAction(MatchContext * context, enum MatchActionType type){
	MatchAction * ret = (MatchAction*) calloc(sizeof(MatchAction), 1);
	ret->type = type;

	// Append new action to enqueue
	MatchAction * end = context->enqueue;
	if(end == NULL)
		context->enqueue = ret;
	else {
		// Append ret to the end of enqueue
		while(end->next != NULL)
			end = end->next;
		end->next = ret;
	}
	return ret;
}

MatchAction * matchEnqueueActorAction(MatchContext * context, enum MatchActionType type, Hunter * actor){
	MatchAction * action = matchEnqueueAction(context, type);
	action->actor = actor;
	return action;
}

void enqueueEndMatchAction(MatchContext * context){
	matchEnqueueAction(context, END_MATCH_ACTION);
}

void enqueueBeginMatchAction(MatchContext * context){
	matchEnqueueAction(context, BEGIN_MATCH_ACTION);
}

void enqueueEndTurnAction(MatchContext * context, Hunter * actor){
	matchEnqueueActorAction(context, TURN_END_ACTION, actor);
}

void enqueueStartTurnAction(MatchContext * context, Hunter * actor){
	matchEnqueueActorAction(context, TURN_START_ACTION, actor);
}

void enqueueRestAction(MatchContext * context, Hunter * actor){
	matchEnqueueActorAction(context, REST_ACTION, actor);
}

void enqueueUseCardAction(MatchContext * context, Hunter * actor, Card * card){
	MatchAction * action = matchEnqueueActorAction(context, USE_CARD_ACTION, actor);
	action->card = card;
}

void enqueueMoveAction(MatchContext * context, Hunter * actor, int x, int y){
	MatchAction * action = matchEnqueueActorAction(context, MOVE_ACTION, actor);
	action->x = x;
	action->y = y;
}

void enqueueMoveStepAction(MatchContext * context, Hunter * actor, int x, int y){
	MatchAction * action = matchEnqueueActorAction(context, MOVE_STEP_ACTION, actor);
	action->x = x;
	action->y = y;
}

void enqueueEndMoveAction(MatchContext * context, Hunter * actor){
	matchEnqueueActorAction(context, END_MOVE_ACTION, actor);
}

void enqueuePollTurnAction(MatchContext * context, Hunter * actor){
	matchEnqueueActorAction(context, POLL_TURN_ACTION, actor);
}

void enqueuePollMoveAction(MatchContext * context, Hunter * actor){
	matchEnqueueActorAction(context, POLL_MOVE_ACTION, actor);
}

void enqueuePollCombatAction(MatchContext * context, Hunter * actor){
	matchEnqueueActorAction(context, POLL_COMBAT_ACTION, actor);
}

void enqueueDrawCardAction(MatchContext * context, Hunter * hunter){
	matchEnqueueActorAction(context, DRAW_CARD_ACTION, hunter);
}

void enqueueDrawCards(MatchContext * context, Hunter * hunter, int number){
	for(int n=0; n < number; n++)
		enqueueDrawCardAction(context, hunter);
}

void enqueueHealAction(MatchContext * context, Hunter * actor, int amount){
	MatchAction * action = matchEnqueueActorAction(context, HEAL_ACTION, actor);
	action->value = amount;
}

void enqueueOpenCrateAction(MatchContext * context, Crate * crate, Hunter * actor){
	MatchAction * action = matchEnqueueActorAction(context, OPEN_CRATE_ACTION, actor);
	action->crate = crate;
}

void enqueueGiveRelicAction(MatchContext * context, Hunter * actor, Relic * relic){
	MatchAction * action = matchEnqueueActorAction(context, GIVE_RELIC_ACTION, actor);
	action->relic = relic;
}

void enqueueEnterCombatAction(MatchContext * context, Hunter * attacker, Hunter * target){
	MatchAction * action = matchEnqueueActorAction(context, ENTER_COMBAT_ACTION, attacker);
	action->target = target;
}

void enqueueExitCombatAction(MatchContext * context){
	matchEnqueueAction(context, EXIT_COMBAT_ACTION);
}

void enqueueAttackAction(MatchContext * context, Hunter * attacker, Hunter * defender){
	MatchAction * action =  matchEnqueueActorAction(context, ATTACK_ACTION, attacker);
	action->target = defender;
}

void enqueueDefendAction(MatchContext * context, Hunter * defender) {
	MatchAction * action =  matchEnqueueActorAction(context, DEFEND_ACTION, defender);
	action->target = defender;
}

void enqueueDealDamageAction(MatchContext * context, Hunter * attacker, Hunter * target, int value){
	MatchAction * action = matchEnqueueActorAction(context, DEAL_DAMAGE_ACTION, target);
	action->actor = attacker;
	action->target = target;
	action->value = (value > 0) ? value : 0;
}

void enqueueDamageAction(MatchContext * context, Hunter * target, int value){
	MatchAction * action = matchEnqueueActorAction(context, DAMAGE_ACTION, target);
	action->target = target;
	action->value = (value > 0) ? value : 0;
}

void enqueueAttackDamageAction(MatchContext * context, Hunter * attacker, Hunter * defender){
	MatchAction * action =  matchEnqueueActorAction(context, ATTACK_DAMAGE_ACTION, attacker);
	action->actor = attacker;
	action->target = defender;
}

void enqueueCombatAction(MatchContext * context, Hunter * attacker, Hunter * defender){
	MatchAction * action =  matchEnqueueActorAction(context, COMBAT_ACTION, attacker);
	action->target = defender;
}

void enqueueDeathCheckAction(MatchContext * context, Hunter * hunter){
	matchEnqueueActorAction(context, DEATH_CHECK_ACTION, hunter);
}

void enqueueEscapeAttemptAction(MatchContext * context){
	matchEnqueueAction(context, ESCAPE_ATTEMPT_ACTION);
}

void enqueueEscapeAction(MatchContext * context){
	matchEnqueueAction(context, ESCAPE_ACTION);
}

void enqueueExecuteCombatAction(MatchContext * context){
	matchEnqueueAction(context, EXECUTE_COMBAT_ACTION);
}

void enqueueMoveRollBonusAction(MatchContext * context, Hunter * actor){
	matchEnqueueActorAction(context, MOVE_ROLL_BONUS_ACTION, actor);
}

void enqueuePollAttackerCardAction(MatchContext * context, Hunter * actor){
	matchEnqueueActorAction(context, POLL_ATTACK_ACTION, actor);
}

void enqueuePollDefenderAction(MatchContext * context, Hunter * actor){
	matchEnqueueActorAction(context, POLL_DEFEND_ACTION, actor);
}
void enqueueRemoveRelicAction(MatchContext * context, Hunter * actor, Relic * relic){
	MatchAction * action = matchEnqueueActorAction(context, REMOVE_RELIC_ACTION, actor);
	action->relic = relic;
}

void enqueueRollDiceAction(MatchContext * context){
	matchEnqueueAction(context, ROLL_DICE_ACTION);
}

void enqueueTeleportAction(MatchContext * context, Hunter * actor, int x, int y){
	MatchAction * action = matchEnqueueActorAction(context, TELEPORT_ACTION, actor);
	action->x = x;
	action->y = y;
}

void enqueueTeleportRandomAction(MatchContext * context, Hunter * actor){
	matchEnqueueActorAction(context, TELEPORT_RANDOM_ACTION, actor);
}
