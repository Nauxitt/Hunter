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

	int dialogue_margin = textures.font.h/2;
	int dialogue_width = textures.font.w*15 + dialogue_margin*2;
	SDL_Rect dest = {
		300 - 10 - dialogue_width,
		game.h - (textures.character_portraits.h*2/3),
		dialogue_width,
		textures.font.h*2 + dialogue_margin*2
	};
	SDL_SetRenderDrawColor(game.renderer, 0,0,0, 255);
	SDL_RenderFillRect(game.renderer, &dest);

	drawString("Hey, you! What", dest.x + dialogue_margin, dest.y + dialogue_margin);
	drawString("do you want?", dest.x + dialogue_margin, dest.y + dialogue_margin + textures.font.h);

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
