#ifndef __brokerState_h
#define __brokerState_h

#include "stateengine.h"
#include "sprites.h"
#include "draw.h"
#include "menubar.h"
#include "statboxDisplayState.h"
#include "scorestate.h"

typedef struct _BrokerState {
	GameState state;
	MenubarState * menubar;
	StatboxDisplayState * statbox;
} BrokerState;

#define BrokerState(s) ((BrokerState*) s)

BrokerState * makeBrokerState(BrokerState * state);
void brokerStateOnDraw(EventHandler * h);
void brokerStateOnKeyUp(EventHandler * h, SDL_Event * e);

typedef struct _SpeechBubbleState {
	GameState state;
	SDL_Rect rect;    // Speech bubble rect.  Height/width set in draw cycle
	int cw, ch;       // Character width/height, in character count
	int margin;       // Number of pixels surrounding text area

	int speed;        // Number of ticks to draw each character.

	char * dialogue;  // String containing dialogue
	int scroll;
} SpeechBubbleState;

SpeechBubbleState * makeSpeechBubbleState(SpeechBubbleState * state, char * dialogue, int x, int y);
void speechBubbleStateOnDraw(EventHandler * h);
void speechBubbleStateOnKeyUp(EventHandler * h, SDL_Event * e);

#endif
