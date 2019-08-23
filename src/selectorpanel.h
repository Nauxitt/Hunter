#ifndef __selectorstate_h
#define __selectorstate_h

#include "hunter.h"
#include "stateengine.h"
#include "sprites.h"
#include "draw.h"

typedef struct _SelectorPanelState SelectorPanelState;
typedef struct _SelectorPanelState {
	GameState state;
	int8_t selector;
	uint8_t cancellable;
	uint8_t select_none;

	void *  source;
	void ** target;

	enum  WindowColor color;
	SDL_Rect rect;
	SDL_Rect icon;

	int8_t max_length;
	int8_t length;
	int item_gutter;
	int panel_margin;

	void (*drawIcon)(SelectorPanelState * state, int n, int x, int y);
	void (*onChoose)(SelectorPanelState * state, int n);
	void (*onCancel)(SelectorPanelState * state);

	void * (*chooseCallback)(SelectorPanelState * state);
} SelectorPanelState;

#define SelectorPanelState(h) ((SelectorPanelState*) h)

SelectorPanelState * makeSelectorPanelState(SelectorPanelState * state);
SelectorPanelState * makeCardSelectState(SelectorPanelState * state, Hunter * hunter, int x, int y);
SelectorPanelState * makeInventorySelectState(SelectorPanelState * state, Hunter * hunter, int x, int y);

void selectorPanelOnTick(EventHandler * h);
void selectorPanelOnDraw(EventHandler * h);
void selectorPanelOnKeyUp(EventHandler * h, SDL_Event * e);
void selectorPanelOnMouseUp(EventHandler * h, SDL_Event * e);

void cardSelectDrawIcon (SelectorPanelState * state, int n, int x, int y);
void cardSelectOnChoose(SelectorPanelState * state, int n);
void relicSelectDrawIcon(SelectorPanelState * state, int n, int x, int y);
void relicSelectOnChoose(SelectorPanelState * state, int n);

#endif
