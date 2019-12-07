#include "nurseState.h"
#include "stateengine.h"
#include "sprites.h"
#include "draw.h"
#include "menubar.h"
#include "scorestate.h"
#include "mainMenuState.h"
#include "brokerState.h"

NurseState * makeNurseState(NurseState * state){
	if(state == NULL)
		state = (NurseState *) calloc(sizeof(NurseState), 1);

	EventHandler(state)->type = "NurseState";
	EventHandler(state)->onPush  = nurseStateOnPush;
	EventHandler(state)->onPop   = nurseStateOnPop;
	EventHandler(state)->onDraw  = nurseStateOnDraw;
	EventHandler(state)->onKeyUp = nurseStateOnKeyUp;

	state->menubar = initMenu(NULL, NULL);
	state->menubar->drawContents = drawMenubarContents;
	state->menubar->selector = 0;
	state->menubar->active = 1;
	state->menubar->length = 2;
	state->menubar->icons[0].id =  2;
	state->menubar->icons[1].id = 16;
	state->menubar->icons[2].id = -1;

	return state;
}

void nurseStateOnDraw(EventHandler * h){
	NurseState * state = NurseState(h);
	drawWallpaper(23);
	spritesheetBlit(
			&textures.character_portraits,
			1,0,
			game.w-textures.character_portraits.w,
			game.h-textures.character_portraits.h
		);

	onDraw(EventHandler(state->menubar));
	onDraw(EventHandler(state->statbox));
}

void nurseStateOnKeyUp(EventHandler * h, SDL_Event * e){
	NurseState * state = NurseState(h);

	switch(e->key.keysym.scancode){
		case SDL_SCANCODE_RETURN:
		case SDL_SCANCODE_SPACE:
			if(state->statbox->hunters_list[state->statbox->selector]){
				Hunter * hunter = state->statbox->hunters_list[state->statbox->selector];
				int cost = hunter->level * 1000;
				// TODO: when gaining credits are implemented, implement cost checking
				// if(hunter->credits <= cost){
				if(1){
					hunter->credits -= cost;
					hunter->level++;
					gamePushState((GameState*) makeStatAllocatorState(
							&state->allocator, hunter, 1
						));
					state->allocator.color = state->statbox->selector;
				}
			}
			break;
		
		case SDL_SCANCODE_TAB:
			onKeyUp(EventHandler(state->statbox), e);
			break;

		case SDL_SCANCODE_LEFT:
		case SDL_SCANCODE_RIGHT:
			onKeyUp(state->menubar, e);
			break;

		case SDL_SCANCODE_ESCAPE:
			gamePopState();
			break;

		default:
			break;
	}
}

void nurseStateOnPush(EventHandler * h){
	SpeechBubbleState * bubble = makeSpeechBubbleState(NULL, "I heal you.", 0, 0);
	bubble->rect.x = game.w-textures.character_portraits.w - 10 - bubble->rect.w;
	bubble->rect.y = game.h - (textures.character_portraits.h*2/3);
	gamePushStateOnTick((GameState*) bubble);
}

void nurseStateOnPop(EventHandler * h){
	mainMenuTransitionIn(MainMenuState(game.state));
}
