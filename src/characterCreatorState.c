#include <stdlib.h>
#include "characterCreatorState.h"
#include "stateengine.h"
#include "characterEntity.h"

CharacterCreatorState * initCharacterCreatorState (CharacterCreatorState * state, Hunter * hunter, int level) {
	if (state == NULL)
		state = malloc(sizeof(CharacterCreatorState));
	memset(state, 0, sizeof(CharacterCreatorState));
	
	if (hunter)
		state->hunter_write = hunter;

	state->mode = 0;
	
	hunter = baseHunter(&state->hunter);
	hunter->level = level;

	EventHandler(state)->onDraw = prevStateOnDraw;
	EventHandler(state)->onTick = characterCreatorOnTick;
	EventHandler(state)->onPush = characterCreatorOnPush;

	return state;
}

void characterCreatorOnPush(EventHandler * h) {
	CharacterCreatorState * creator = (CharacterCreatorState*) h;

	creator->hunter.id = creator->hunter_id;
	creator->window_color = creator->hunter_id;
}

void characterCreatorOnTick(EventHandler * h) {
	CharacterCreatorState * creator = (CharacterCreatorState*) h;

	Hunter * hunter = &creator->hunter;

	switch(creator->mode++) {
		case CHARACTER_CREATOR_MODE_INIT:
			// advance to the next mode by recursing
			characterCreatorOnTick(h);
			break;

		case CHARACTER_CREATOR_MODE_NAME:
			// TODO: prompt user for name
			hunterSetRandomName(hunter, 5);
			characterCreatorOnTick(h);
			break;

		case CHARACTER_CREATOR_MODE_AVATAR:
			// Push avatar selector state
			gamePushState((GameState*) initAvatarSelectorState(
					&creator->avatar_state, &creator->hunter.avatar, NULL
				));
			break;

		case CHARACTER_CREATOR_MODE_STATS:
			// Push stat allocator state
			hunter->level = 1;
			gamePushState((GameState*) makeStatAllocatorState(
					&creator->allocator_state, hunter, 10
				));
			creator->allocator_state.color = creator->window_color;
			break;

		case CHARACTER_CREATOR_MODE_CONFIRM:
			// TODO: confirm the created character with the user
			characterCreatorOnTick(h);
			break;

		default:
		case CHARACTER_CREATOR_MODE_END:
			// If there's a hunter to write to, copy the new hunter over.
			if (creator->hunter_write)
				memcpy(
						creator->hunter_write,
						&creator->hunter,
						sizeof(Hunter)
					);

			gamePopState();
			break;
	}
}

AvatarSelectorState * initAvatarSelectorState (AvatarSelectorState * state, int * avatar_write, int * color_write) {
	if (state == NULL)
		state = malloc(sizeof(AvatarSelectorState));
	memset(state, 0, sizeof(AvatarSelectorState));

	state->avatar_write = avatar_write;
	state->color_write  = color_write;

	CharacterAnimationHandler * handler;

	for (int id=0; id < CHARACTER_ID_MAX; id++) {
		// Ensure that each handler is loaded. They should be freed upon
		// exiting this state.
		handler = getCharacterAnimationHandler(id);
	}

	state->rect.x = 100;
	state->rect.y = 100;
	state->rect.w = 200;
	state->rect.h = 32 + handler->tile_h * handler->scale_h;

	EventHandler(state)->onKeyUp = avatarSelectorStateOnKeyUp;
	EventHandler(state)->onDraw  = avatarSelectorStateOnDraw;

	return state;
}

void avatarSelectorStateOnKeyUp(EventHandler * h, SDL_Event * e) {
	AvatarSelectorState * state = (AvatarSelectorState*) h;

	switch(e->key.keysym.scancode) {
		// Up and down keys cycle through avatars

		case SDL_SCANCODE_UP:
			if (--state->avatar < 0)
				state->avatar = CHARACTER_ID_MAX - 1;
			break;

		case SDL_SCANCODE_DOWN:
			if (++state->avatar >= CHARACTER_ID_MAX)
				state->avatar = 0;
			break;

		/*
		// Left and right keys cycle through colors

		case SDL_SCANCODE_LEFT:
			if (--state->color <= 0)
				;
			break;

		case SDL_SCANCODE_RIGHT:
			if (++state->color >= 0)
				;
			break;
		*/
		
		case SDL_SCANCODE_ESCAPE:
			// TODO: cancel
			break;
			
		case SDL_SCANCODE_SPACE:
		case SDL_SCANCODE_RETURN:
			avatarSelectorStateEnd(state);
			break;

		default:
			break;
	}
}

void avatarSelectorStateEnd(AvatarSelectorState * state) {
	for (int id=0; id < CHARACTER_ID_MAX; id++) {
		// Free animation handlers, or at least deincrement references to
		// them. This prevents frequent allocating and deallocating of the
		// animation handler with each user input by keeping at least one 
		// reference alive.
		freeCharacterAnimationHandler(id);
	}

	if (state->avatar_write)
		*state->avatar_write = state->avatar;

	if (state->color_write)
		*state->color_write  = state->color;

	gamePopState();
}

void avatarSelectorStateOnDraw(EventHandler * h) {
	AvatarSelectorState * state = (AvatarSelectorState*) h;
	prevStateOnDraw(GameState(h));

	// Animate panel expansion, exit if not finished
	float expand_time = 175;
	float scale = (float) GameState(state)->duration / expand_time;
	if (scale < 1.0) {
		drawWindowPanelScaled(state->color, &state->rect, scale);
		return;
	}
	drawWindowPanel(state->window_color, &state->rect);
	
	// TODO: Draw avatar arrows
	
	// Draw avatar using spritesheet metadta from the animation handler
	
	CharacterAnimationHandler * handler = getCharacterAnimationHandler(state->avatar);

	SDL_Rect src = {0, 0, handler->tile_w, handler->tile_h};

	SDL_Rect dest = {
			state->rect.x + 8,
			
			// Vertically center in panel
			state->rect.y + state->rect.h/2 - handler->tile_h * handler->scale_h / 2,

			handler->tile_w * handler->scale_w,
			handler->tile_h * handler->scale_h
		};

	blit(handler->texture, &src, &dest);
	
	freeCharacterAnimationHandler(state->avatar);
	
	
	// TODO: Draw color selectors
}

#include "stateengine.h"
int characterCreatorMain(int argc, char ** argv) {
	CharacterCreatorState creator;
	Hunter hunter;

	initGame();
	loadSprites();

	initCharacterCreatorState(&creator, &hunter, 10);
	gamePushState((GameState*) &creator);
	gameMainLoop();

	printHunter(&hunter);

	return 0;
}
