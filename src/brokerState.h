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
void brokerStateOnPush(EventHandler * h);
void brokerStateOnPop(EventHandler * h);
void brokerStateOnDraw(EventHandler * h);
void brokerStateOnKeyUp(EventHandler * h, SDL_Event * e);

typedef struct _SpeechBubbleState {
	GameState state;
	SDL_Rect rect;    // Speech bubble rect.  Height/width set in draw cycle
	int cw, ch;       // Character width/height, in character count
	int margin;       // Number of pixels surrounding text area
	int pulse_margin;

	int speed;        // Number of ticks to draw each character.
	int pulse_speed;  // Speed of one pulse of the speech bubble
	int skip;         // If true, skips individual-character drawing

	char * dialogue;  // String containing dialogue
	int scroll;
} SpeechBubbleState;

SpeechBubbleState * makeSpeechBubbleState(SpeechBubbleState * state, char * dialogue, int x, int y);
void speechBubbleUpdateSize(SpeechBubbleState * state);
void speechBubbleStateOnDraw(EventHandler * h);
void speechBubbleStateOnKeyUp(EventHandler * h, SDL_Event * e);

#endif
