#include "brokerState.h"
#include "stateengine.h"
#include "sprites.h"
#include "draw.h"
#include "menubar.h"
#include "scorestate.h"
#include "mainMenuState.h"

BrokerState * makeBrokerState(BrokerState * state){
	if(state == NULL)
		state = (BrokerState *) calloc(sizeof(BrokerState), 1);

	EventHandler(state)->type = "BrokerState";
	EventHandler(state)->onDraw = brokerStateOnDraw;
	EventHandler(state)->onKeyUp = brokerStateOnKeyUp;
	return state;
}

void brokerStateOnDraw(EventHandler * h){
	BrokerState * state = BrokerState(h);
	onDraw(EventHandler(state->statbox));
}

void brokerStateOnKeyUp(EventHandler * h, SDL_Event * e){
	BrokerState * state = BrokerState(h);

	switch(e->key.keysym.scancode){
		case SDL_SCANCODE_RETURN:
		case SDL_SCANCODE_SPACE:
			mainMenuStartBasicMission(MainMenuState(GameState(state)->prevState));
			break;

		case SDL_SCANCODE_ESCAPE:
			gamePopState();
			break;

		default:
			break;
	}
}
