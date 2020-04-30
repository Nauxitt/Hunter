#ifndef __characterCreatorState_h
#define __characterCreatorState_h

#include "stateengine.h"
#include "statAllocatorPanel.h"

/*
 * Used to indicate which of the several dialogues that consist character
 * creation the creator state is on.  These are incremented to advance the
 * creation process.
 */
enum CharacterCreatorMode {
	CHARACTER_CREATOR_MODE_INIT,
	CHARACTER_CREATOR_MODE_NAME,
	CHARACTER_CREATOR_MODE_STATS,
	CHARACTER_CREATOR_MODE_AVATAR,
	CHARACTER_CREATOR_MODE_CONFIRM,
	CHARACTER_CREATOR_MODE_END
};

typedef struct _AvatarSelectorState {
	GameState * state;

	int avatar;
	int color;

	int * avatar_write;
	int * color_write;
} AvatarSelectorState;

/*
 * The CharacterCreatorState manages a sequence of GameStates, which prompt the
 * user on the cusomization of their character.  It holds the date between these
 * states, but does not display any content or directly interact with the user,
 * itself.
 */
typedef struct _CharacterCreatorState {
	GameState state;

	enum WindowColor color;
	Hunter hunter;
	Hunter * hunter_write;

	enum CharacterCreatorMode mode;
	
	StatAllocatorState allocator;
	//NamePromptState name_prompt;
	AvatarSelectorState avatar;
} CharacterCreatorState;


CharacterCreatorState * makeCharacterCreatorState(CharacterCreatorState * state, Hunter * hunter);
void characterCreatorOnTick(EventHandler * h);
 
AvatarSelectorState * makeAvatarSelectorState(AvatarSelectorState * state);
void avatarSelectorStateOnKeyUp(EventHandler * h, SDL_Event * e);
void avatarSelectorStateOnTick(EventHandler * h);
void avatarSelectorStateOnDraw(EventHandler * h);
void avatarSelectorStateEnd(AvatarSelectorState * state);

int characterCreatorMain(int argc, char ** argv);

#endif
