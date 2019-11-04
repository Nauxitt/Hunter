#include "stateengine.h"
# include "statboxDisplayState.h"
#include "draw.h"
#include "hunter.h"

StatboxDisplayState * makeStatboxDisplayState(StatboxDisplayState * state){
	if(state == NULL)
		state = (StatboxDisplayState *) calloc(sizeof(StatboxDisplayState), 1);

	EventHandler(state)->type = "StatboxDisplayState";
	EventHandler(state)->onDraw = statboxDisplayStateOnDraw;
	EventHandler(state)->onKeyUp = statboxDisplayStateOnKeyUp;

	return state;
}

void statboxDisplayStateOnDraw(EventHandler * h){
	StatboxDisplayState * state = StatboxDisplayState(h);

	// statboxes
	int panel_gutter = 4;
	int panel_w = (game.w - 16*2 - 4*3) / 4;
	for(int h=0; h < 4; h++){
		Hunter * hunter = state->hunters_list[h];

		if(hunter == NULL)
			continue;

		drawStatbox(
				hunter,
				(enum StatboxViews) state->view,
				(enum WindowColor) h,
				16 + (panel_w+panel_gutter)*h,
				game.h-160-panel_gutter
			);
	}
}

void statboxDisplayStateOnKeyUp(EventHandler * h, SDL_Event * e){
	StatboxDisplayState * state = StatboxDisplayState(h);

	// Toggle statboxron TAB
	if(e->key.keysym.scancode == SDL_SCANCODE_TAB){
		state->view++;
		if(state->view == STATBOX_VIEW_NONE + 1)
			state->view = 0;
	}
}
