#ifndef __nurseState_h
#define __nurseState_h

#include "stateengine.h"
#include "sprites.h"
#include "draw.h"
#include "menubar.h"
#include "statboxDisplayState.h"
#include "scorestate.h"
#include "statAllocatorPanel.h"

typedef struct _NurseState {
	GameState state;
	MenubarState * menubar;
	StatAllocatorState allocator;
	StatboxDisplayState * statbox;
} NurseState;

#define NurseState(s) ((NurseState*) s)

NurseState * makeNurseState(NurseState * state);
void nurseStateOnPush(EventHandler * h);
void nurseStateOnPop(EventHandler * h);
void nurseStateOnDraw(EventHandler * h);
void nurseStateOnKeyUp(EventHandler * h, SDL_Event * e);

#endif
