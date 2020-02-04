#include <SDL2/SDL_ttf.h>

#include "hunter.h"
#include "combatstate.h"
#include "selectorpanel.h"
#include "mapstate.h"
#include "dicestate.h"
#include "sprites.h"

CombatState * makeCombatState(CombatState * state, MatchContext * match) {
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

void combatOnEnter(EventHandler * h) {
	CombatState * state = CombatState(h);
	state->menubar->active = 0;
}

void combatOnExit(EventHandler * h) {
	// CombatState * state = CombatState(h);
}

void combatOnTick(EventHandler * h) {
	CombatState * state = CombatState(h);
	MatchContext * match = state->match;
	
	// Handle relic selection
	if(pollAction("surrender_relic_select")){
		nextAction();
		if(state->relic_selected != NULL){
			postSurrenderAction(match, state->relic_selected);
		}
	}

	int breaker = 1;
	while((breaker == 1) && (match->action)){
		MatchAction * action = match->action;
		
		HunterEntity * actor_entity = &state->attacker_entity;
		if(actor_entity->hunter != action->actor)
			actor_entity = &state->defender_entity;
		
		HunterEntity * target_entity = &state->attacker_entity;
		if(target_entity->hunter != action->target)
			target_entity = &state->defender_entity;

		switch(action->type){
			case POLL_DEFEND_ACTION:
			case POLL_COMBAT_CARD_ACTION:
			case POLL_COMBAT_ACTION:
				breaker = 0;
				matchCycle(match);
				break;

			case POLL_ATTACK_ACTION:
				matchCycle(match);

				if (action->type != POLL_ATTACK_ACTION)
					break;

				if(state->card_selected == NULL){
					breaker = 0;
					SelectorPanelState * handState = makeCardSelectState(NULL, action->actor, 32, 72);
					handState->target = (void*) &state->card_selected;
					gamePushState(GameState(handState));
					return;
				}
				else {
					postAttackerCard(match, state->card_selected);
				}
				break;

			case DAMAGE_ACTION:
				gamePushState((GameState*) makeHunterEntityDamageState(
						NULL, target_entity, action->value
					));
				matchCycle(match);
				return;

			case ROLL_DICE_ACTION:
				matchCycle(match);
				gamePushState((GameState*) initDiceState(
							NULL, match->dice[0],
							game.w/2 - textures.dice.w*3/2,
							game.h - textures.dice.h *3/2,
							DAMAGE_DICE_COLOR
						));
				gamePushState((GameState*) initDiceState(
							NULL, match->dice[1],
							game.w/2 - textures.dice.w/2,
							game.h - textures.dice.h *3/2,
							DAMAGE_DICE_COLOR
						));
				gamePushState((GameState*) initDiceState(
							NULL, match->dice[2],
							game.w/2 + textures.dice.w/2,
							game.h - textures.dice.h *3/2,
							DEFENSE_DICE_COLOR
						));
				gamePushState((GameState*) initDiceState(
							NULL, match->dice[3],
							game.w/2 + textures.dice.w*3/2,
							game.h - textures.dice.h *3/2,
							DEFENSE_DICE_COLOR
						));
				return;

			case USE_CARD_ACTION:
			case DEAL_DAMAGE_ACTION:
			case ATTACK_DAMAGE_ACTION:
			case ENTER_COMBAT_ACTION:
			case MOVE_ROLL_BONUS_ACTION:
			case CATCH_ROLL_BONUS_ACTION:
			case ESCAPE_ROLL_BONUS_ACTION:
			case ATTACK_ROLL_BONUS_ACTION:
			case DEFENSE_ROLL_BONUS_ACTION:
			case EXECUTE_COMBAT_ACTION:
			case ATTACK_ACTION:
			case DEFEND_ACTION:
			case ESCAPE_ACTION:
			case ESCAPE_ATTEMPT_ACTION:
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
				return;
		}
	}
}

void combatOnDraw(EventHandler * h) {
	CombatState * state = CombatState(h);
	MatchContext * match = state->match;

	onDraw(EventHandler(&state->attacker_entity));
	onDraw(EventHandler(&state->defender_entity));

	int panel_gutter = 4;
	int panel_w = (game.w - 16*2 - 4*3) / 4;

	drawStatbox(
			match->attacker,
			(enum StatboxViews) 0,
			(enum WindowColor) match->attacker->id,
			16,
			game.h-160-panel_gutter
		);

	drawStatbox(
			match->defender,
			(enum StatboxViews) 0,
			(enum WindowColor) match->defender->id,
			16 + (panel_w+panel_gutter) * 3,
			game.h-160-panel_gutter
		);

	if(match->action->type == POLL_DEFEND_ACTION){
		SDL_Color white = {255, 255, 255};

		char * actionName;
		switch(state->selector){
			case 0: actionName = "Defender Action:  Attack";    break;
			case 1: actionName = "Defender Action:  Defend";    break;
			case 2: actionName = "Defender Action:  Run";       break;
			case 3: actionName = "Defender Action:  Surrender"; break;
		}

		SDL_Surface * surfaceMessage = TTF_RenderText_Solid(game.font, actionName, white);
		SDL_Texture * messageTexture = SDL_CreateTextureFromSurface(game.renderer, surfaceMessage);
		SDL_Rect dest = {100, 100, 0, 0};
		SDL_QueryTexture(
				messageTexture,
				NULL, NULL,
				&dest.w, &dest.h
			);

		SDL_RenderCopy(game.renderer, messageTexture, NULL, &dest);
	}

	// Forward event to menubar
	if(state->menubar)
		onDraw(EventHandler(state->menubar));
}

void combatOnKeyUp(EventHandler * h, SDL_Event * e){
	CombatState * state = CombatState(h);
	MatchContext * match = state->match;

	// Escape prints a debug message of the current match action
	if(e->key.keysym.scancode == SDL_SCANCODE_BACKSPACE){
		printf("Backspace: ");
		printMatchAction(match->action);
	}

	if(match->action->type == POLL_DEFEND_ACTION){
		switch(e->key.keysym.scancode){
			case SDL_SCANCODE_LEFT:
				state->selector--;
				if(state->selector < 0)
					state->selector = 3;
				break;

			case SDL_SCANCODE_RIGHT:
				state->selector++;
				if(state->selector >= 4)
					state->selector = 0;
				break;

			case SDL_SCANCODE_SPACE:
			case SDL_SCANCODE_RETURN:
				switch(state->selector){
					case 0: postDefenderAction(match, ATTACK_ACTION, NULL); break;
					case 1: postDefenderAction(match, DEFEND_ACTION, NULL); break;
					case 2: postDefenderAction(match, ESCAPE_ACTION, NULL); break;
					case 3:
						if(hunterInventoryLength(state->defender_entity.hunter) >= 1){
							SelectorPanelState * relicSelect = makeInventorySelectState(
									NULL, state->defender_entity.hunter, 32, 72
								);

							relicSelect->target = (void*) &state->relic_selected;
							relicSelect->select_none = 0;

							gamePushState(relicSelect);
							pushAction("surrender_relic_select");
						}

						break;
				}
				state->selector = 0;
				break;

			default:
				break;
		}
	}
}
