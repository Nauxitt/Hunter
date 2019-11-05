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

	state->menubar = initMenu(NULL, NULL);
	state->menubar->drawContents = drawMenubarContents;
	state->menubar->selector = 0;
	state->menubar->active = 1;
	state->menubar->length = 4;
	state->menubar->icons[4].id = -1;

	return state;
}

void brokerStateOnDraw(EventHandler * h){
	BrokerState * state = BrokerState(h);
	drawWallpaper(22);
	spritesheetBlit(&textures.character_portraits, 0,0, 300,game.h-textures.character_portraits.h);
	onDraw(EventHandler(state->menubar));
	onDraw(EventHandler(state->statbox));
}

void brokerStateOnKeyUp(EventHandler * h, SDL_Event * e){
	BrokerState * state = BrokerState(h);

	switch(e->key.keysym.scancode){
		case SDL_SCANCODE_RETURN:
		case SDL_SCANCODE_SPACE:
			gamePopState();
			mainMenuStartBasicMission(MainMenuState(GameState(state)->prevState));
			return;

		case SDL_SCANCODE_ESCAPE:
			gamePopState();
			break;

		default:
			break;
	}
}
