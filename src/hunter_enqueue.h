#ifndef __hunter_enqueue_h
#define __hunter_enqueue_h

#include "hunter.h"

MatchAction * matchEnqueueAction(MatchContext * context, enum MatchActionType type);
MatchAction * matchEnqueueActorAction(MatchContext * context, enum MatchActionType type, Hunter * actor);

void enqueueBeginMatchAction(MatchContext * context);
void enqueueEndTurnAction(MatchContext * context, Hunter * actor);
void enqueueStartTurnAction(MatchContext * context, Hunter * actor);
void enqueueMoveAction(MatchContext * context, Hunter * actor, int x, int y);
void enqueueEndMoveAction(MatchContext * context, Hunter * actor);
void enqueuePollTurnAction(MatchContext * context, Hunter * actor);
void enqueueDrawCardAction(MatchContext * context, Hunter * hunter);
void enqueueDrawCards(MatchContext * context, Hunter * hunter, int number);
void enqueueHealAction(MatchContext * context, Hunter * actor, int amount);
void enqueueOpenCrateAction(MatchContext * context, Crate * crate, Hunter * actor);
void enqueueGiveRelicAction(MatchContext * context, Hunter * actor, Relic * relic);
void enqueueRemoveRelicAction(MatchContext * context, Hunter * actor, Relic * relic);
void enqueueEnterCombatAction(MatchContext * context, Hunter * attacker, Hunter * defender);
void enqueueExitCombatAction(MatchContext * context);
void enqueueDeathCheckAction(MatchContext * context, Hunter * hunter);
void enqueueRollDiceAction(MatchContext * context);

void enqueueMoveStepAction(MatchContext * context, Hunter * actor, int x, int y);
void enqueueTeleportRandomAction(MatchContext * context, Hunter * actor);
void enqueueTeleportAction(MatchContext * context, Hunter * actor, int x, int y);
void enqueueUseCardAction(MatchContext * context, Hunter * actor, Card * card);
void enqueueRestAction(MatchContext * context, Hunter * actor);

void enqueueMoveRollBonusAction(MatchContext * context, Hunter * actor);
void enqueueCatchRollBonusAction(MatchContext * context, Hunter * actor);
void enqueueEscapeRollBonusAction(MatchContext * context, Hunter * actor);
void enqueueAttackRollBonusAction(MatchContext * context, Hunter * actor);
void enqueueDefenseRollBonusAction(MatchContext * context, Hunter * actor);

void enqueueCombatAction(MatchContext * context, Hunter * attacker, Hunter * defender);
void enqueuePollMoveAction(MatchContext * context, Hunter * actor);
void enqueuePollCombatAction(MatchContext * context, Hunter * actor);
void enqueuePollDefenderAction(MatchContext * context, Hunter * actor);
void enqueuePollAttackerCardAction(MatchContext * context, Hunter * actor);
void enqueueExecuteCombatAction(MatchContext * context);
void enqueueEscapeAttemptAction(MatchContext * context);
void enqueueAttackAction(MatchContext * context, Hunter * attacker, Hunter * defender);
void enqueueAttackDamageAction(MatchContext * context, Hunter * attacker, Hunter * defender);
void enqueueDamageAction(MatchContext * context, Hunter * target, int value);

#endif
