#include <stdlib.h>
#include "characterCreatorState.h"
#include "stateengine.h"

CharacterCreatorState * makeCharacterCreatorState(CharacterCreatorState * state, Hunter * hunter) {
	if (state == NULL)
		state = malloc(sizeof(CharacterCreatorState));
	memset(state, 0, sizeof(CharacterCreatorState));
	
	if (hunter)
		state->hunter_write = hunter;

	state->mode = 0;

	EventHandler(state)->onDraw = prevStateOnDraw;
	EventHandler(state)->onTick = characterCreatorOnTick;

	return state;
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
			characterCreatorOnTick(h);
			break;

		case CHARACTER_CREATOR_MODE_STATS:
			// Push stat allocator state
			hunter = randomHunter(hunter, 0);
			hunter->level = 1;
			gamePushState((GameState*) makeStatAllocatorState(
					&creator->allocator, hunter, 10
				));
			creator->allocator.color = creator->color;
			break;

		case CHARACTER_CREATOR_MODE_AVATAR:
			// TODO: Push avatar selector state
			characterCreatorOnTick(h);
			gamePushState((GameSTate*) makeAvatarSelectorState(
					&creator->avatar, hunter
				));
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

AvatarSelectorState * makeAvatarSelectorState(AvatarSelectorState * state) {
	if (state == NULL)
		state = calloc(sizeof(AvatarSelectorState), 1);

	state->avatar = 0;
	state->color = 0;

	return state;
}

void avatarSelectorStateOnKeyUp(EventHandler * h, SDL_Event * e) {
	AvatarSelectorState * state = (AvatarSelectorState*) h;

	switch(e->key.keysym.scancode) {
		case SDL_SCANCODE_UP:
			if (--state->avatar <= 0)
				;
			break;

		case SDL_SCANCODE_DOWN:
			if (++state->avatar >= 0)
				;
			break;

		case SDL_SCANCODE_LEFT:
			if (--state->color <= 0)
				;
			break;

		case SDL_SCANCODE_RIGHT:
			if (++state->color >= 0)
				;
			break;
		
		case SDL_SCANCODE_ESCAPE:
			// TODO: cancel
			
		case SDL_SCANCODE_RETURN:
			avatarSelectorStateEnd(state);
			break;

		default:
			break;
	}
}

void avatarSelectorStateEnd(AvatarSelectorState * state) {
	*state->avatar_write = state->avatar;
	*state->color_write  = state->color;

	gamePopState();
}

void avatarSelectorStateOnDraw(EventHandler * h) {
	prevStateOnDraw(h);
}

#include "stateengine.h"
int characterCreatorMain(int argc, char ** argv) {
	CharacterCreatorState creator;
	Hunter hunter;

	initGame();
	loadSprites();

	makeCharacterCreatorState(&creator, &hunter);
	gamePushState((GameState*) &creator);
	gameMainLoop();

	printHunter(&hunter);

	return 0;
}
