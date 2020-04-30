#include "stateengine.h"
#include "mapstate.h"
#include "characterEntity.h"

GameState characterEntityAnimationBlockState = {.events = {
	.data = NULL,
	.type = "CharacterEntityBlockState",
	.onTick = characterEntityAnimationBlockTicksStateOnTick,
	.onDraw = prevStateOnDraw,
}};

void characterEntityAnimationBlockTicks(HunterEntity * entity) {
	EventHandler(&characterEntityBlockState)->data = entity->animation_context;
	gamePushState((GameState *) &characterEntityAnimationBlockState);
}

void characterEntityAnimationBlockTicksStateOnTick(EventHandler * h) {
	if (((CharacterEntityAnimationContext *) h->data)->finished)
		gamePopState();
}
