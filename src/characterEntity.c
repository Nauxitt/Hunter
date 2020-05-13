#include "stateengine.h"
#include "mapstate.h"
#include "characterEntity.h"
#include "entity.h"
#include "animations.h"
#include "sprites.h"

CharacterAnimationHandler * characterAnimationHandlers[CHARACTER_ID_MAX];

GameState characterAnimationBlockState = {.events = {
	.data = NULL,
	.type = "CharacterBlockState",
	.onTick = characterAnimationBlockTicksStateOnTick,
	.onDraw = prevStateOnDraw,
}};

void characterAnimationBlockTicks(HunterEntity * entity) {
	EventHandler(&characterBlockState)->data = &entity->animation_context;
	gamePushState((GameState *) &characterAnimationBlockState);
}

void characterAnimationBlockTicksStateOnTick(EventHandler * h) {
	if (((CharacterAnimationContext *) h->data)->finished)
		gamePopState();
}

void characterEntityOnDraw(EventHandler * h) {
	HunterEntity * character = (HunterEntity*) h;
	CharacterAnimationHandler * a_handler = character->animation_handler;
	CharacterAnimationContext * a_context = &character->animation_context;

	// Update animations on directional changes

	if (a_context == NULL)
		; // Prevent segfaults during if expressions
	else if (a_context->direction != a_context->last_direction) {
		a_context->last_direction = a_context->direction;
		animationContextSetCharacterAnimation(a_context, a_context->animation_current);
	}
	else if (a_context->direction != Entity(character)->direction) {
		a_context->last_direction = Entity(character)->direction;
		a_context->direction      = Entity(character)->direction;
		animationContextSetCharacterAnimation(a_context, a_context->animation_current);
	}
	

	// Draw the character, then prepare character
	// animation logic for the next tick.
	tileEntityOnDraw(h);

	// If there is insufficient data for the CharacterEntity animation system,
	// default to the base Entity animation system by exiting the function.
	if (a_handler == NULL)
		return;

	if (a_context->animation_current == NULL)
		return;

	/*
	   Handle animation finishing/looping
	*/
	if (Entity(character)->animation_finished) {
		CharacterAnimation * next_animation = a_context->animation_current->next;

		if (next_animation == NULL) {
			if (a_context->loop) {
				animationContextSetCharacterAnimation(a_context, a_context->animation_start);
			}
			else {
				a_context->finished = 1;
			}
		}
		else {
			a_context->animation_current = next_animation;
			animationContextSetCharacterAnimation(a_context, next_animation);
		}
	}
}

void characterLoopAnimation (HunterEntity * character, enum CharacterAnimationType animation_id) {
	characterSetAnimation(character, animation_id);
	character->animation_context.loop = 1;
}

void characterSetAnimation (HunterEntity * character, enum CharacterAnimationType animation_id) {
	CharacterAnimationHandler * a_handler = character->animation_handler;
	CharacterAnimationContext * a_context = &character->animation_context;

	a_context->loop = 0;
	
	CharacterAnimation * animation = a_handler->animations[animation_id];
	
	if (animation == NULL) {
		animation = a_handler->animations[0];

		if (animation == NULL)
			return;  // TODO: raise exception
	}

	printf("Anim set (%s): %d, %x\n", character->hunter->name, animation_id, (int)animation);
	animationContextSetCharacterAnimation(a_context, animation);
}

CharacterAnimationContext * initCharacterAnimationContext(CharacterAnimationContext * context, CharacterAnimationHandler * handler, HunterEntity * character) {
	if (context == NULL)
		context = malloc(sizeof(CharacterAnimationContext));

	memset(context, 0, sizeof(CharacterAnimationContext));

	context->handler = handler;
	context->character = character;

	return context;
}

void animationContextSetCharacterAnimation(CharacterAnimationContext * a_context, CharacterAnimation * c_animation) {
	// Set context data: finished, animation_current
	a_context->finished = 0;
	a_context->animation_current = c_animation;
	a_context->animation_start = c_animation;

	// Get animation directional data and respective animation
	Entity * entity = (Entity*) a_context->character;
	
	AnimationFrame * directional_animation = NULL;
	AnimationFrame * anim_n = c_animation->entity_animation_n;
	AnimationFrame * anim_s = c_animation->entity_animation_s;

	// Detect directional changes

	if (a_context->direction != a_context->last_direction) {
		a_context->last_direction = a_context->direction;
	}
	else if (a_context->direction != entity->direction) {
		a_context->last_direction = entity->direction;
		a_context->direction      = entity->direction;
	}

	entity->direction = a_context->direction;

	switch (a_context->direction) {
		case NORTH:
			entity->flip_h = 0;
			directional_animation = anim_n;
			break;

		case EAST:
			entity->flip_h = 1;
			directional_animation = anim_s;
			break;

		case SOUTH:
			entity->flip_h = 0;
			directional_animation = anim_s;
			break;

		case WEST:
			entity->flip_h = 1;
			directional_animation = anim_n;
			break;
	}
	
	// If the corresponding directional animation is NULL, default to the other
	if (directional_animation == 0) {
		directional_animation = (AnimationFrame *) ((uintptr_t) anim_n | (uintptr_t) anim_s);
		
		if (directional_animation == 0) {
			// TODO: throw exception
			return;
		}
	}
	
	if (a_context->loop == 0)
		entity->animation_loop = 0;

	if (entity->animation_finished)
		entity->animation = NULL;

	entitySetAnimation(entity, directional_animation);
}

int animationHandlerGetAnimationDuration (CharacterAnimationHandler * handler, enum CharacterAnimationType id) {
	CharacterAnimation * animation = handler->animations[id];
	int duration = 0;
	for (; animation; animation = animation->next) {
		int duration1 = animationGetDuration(animation->entity_animation_n);
		int duration2 = animationGetDuration(animation->entity_animation_s);

		int max_duration = (duration1 > duration2) ? duration1 : duration2;

		if (max_duration == 0 && animation->next == NULL && animation->redirect)
			animationHandlerGetAnimationDuration(handler, animation->redirect);
		else
			duration += max_duration;
	}
	return duration;
}

AnimationFrame * generateCharacterAnimationFrames (CharacterAnimationHandler * handler, CharacterAnimationSlice * slice) {
	if (slice == NULL || slice->length == 0)
		return NULL;

	AnimationFrame * animation = calloc(sizeof(AnimationFrame), slice->length);

	for (int i=0; i < slice->length; i++) {
		AnimationFrame * frame = animation + i;

		// Animation linked list (NULL-terminate after loop)
		frame->next = frame + 1;

		frame->clip.x = handler->tile_w * (slice->col + i);
		frame->clip.y = handler->tile_h * slice->row;
		frame->clip.w = handler->tile_w;
		frame->clip.h = handler->tile_h;
		
		frame->center_x = handler->center_x;
		frame->center_y = handler->center_y;
		
		frame->duration = slice->duration;
	};
	
	// NULL-terminate final frame
	animation[slice->length - 1].next = NULL;

	return animation;
}

CharacterAnimation * generateCharacterAnimation (CharacterAnimationHandler * handler, CharacterAnimationSlice * slice_n, CharacterAnimationSlice * slice_s) {
	CharacterAnimation * animation = calloc(sizeof(CharacterAnimation), 1);

	animation->entity_animation_n = generateCharacterAnimationFrames(handler, slice_n);
	animation->entity_animation_s = generateCharacterAnimationFrames(handler, slice_s);

	return animation;
}

CharacterAnimationHandler * getCharacterAnimationHandler (enum AvatarId id) {
	// Lazy loading of handlers
	if (characterAnimationHandlers[id]) {
		CharacterAnimationHandler * handler = characterAnimationHandlers[id];

		handler->references++;

		return handler;
	}
	
	// Hanlder not initialized, so allocate and init a new
	// one.
	CharacterAnimationHandler * handler = calloc(sizeof(CharacterAnimationHandler), 1);

	handler->id = id;

	// Default scaling and grid size values
	handler->scale_w = 2;
	handler->scale_h = 2;

	handler->tile_w = 42;
	handler->tile_h = 42;

	handler->center_x = handler->tile_w / 2;
	handler->center_y = handler->tile_h - 6;

	// Default animation tile locations
	handler->define_stand_n.row = 0;
	handler->define_stand_s.row = 0;

	handler->define_hit_n.row = 1;
	handler->define_hit_s.row = 1;

	handler->define_taunt_n.row = 2;
	handler->define_taunt_s.row = 2;
	handler->define_ko_n.row = 3;
	handler->define_ko_s.row = 3;

	handler->define_run_s.row = 4;
	handler->define_run_n.row = 5;

	// Most default north animations occupy the same row as their
	// southern counterpart, but start at the fifth column.
	
	handler->define_stand_n.col = 4;
	handler->define_hit_n.col = 4;
	handler->define_taunt_n.col = 4;
	handler->define_ko_n.col = 4;

	// Default animation durations
	
	handler->define_stand_n.duration = 240;
	handler->define_stand_s.duration = 240;

	handler->define_hit_n.duration = 120;
	handler->define_hit_s.duration = 120;

	handler->define_taunt_n.duration = 140;
	handler->define_taunt_s.duration = 140;

	handler->define_ko_n.duration = 750;
	handler->define_ko_s.duration = 750;

	handler->define_run_s.duration = 20;
	handler->define_run_n.duration = 20;

	// Default animation lengths
	
	handler->define_stand_n.length = 3;
	handler->define_stand_s.length = 3;

	handler->define_hit_n.length = 2;
	handler->define_hit_s.length = 2;

	handler->define_taunt_n.length = 3;
	handler->define_taunt_s.length = 3;

	handler->define_ko_n.length = 3;
	handler->define_ko_s.length = 3;

	handler->define_run_s.length = 5;
	handler->define_run_n.length = 5;

	switch (id) {
		case CHARACTER_DANIEL:
			handler->name = "Daniel";
			handler->texture = textures.daniel.texture;
			break;

		case CHARACTER_CHRISTINA:
			handler->name = "Christina";
			handler->texture = textures.christina.texture;
			handler->define_stand_n.col++;
			handler->define_stand_s.col++;
			break;

		case CHARACTER_SIMON:
			handler->name = "Simon";
			handler->texture = textures.simon.texture;
			handler->define_stand_n.col++;
			handler->define_stand_s.col++;
			handler->define_taunt_s.length = 4;
			handler->define_taunt_n.length = 4;
			handler->define_run_s.length = 6;
			handler->define_run_n.length = 6;
			break;

		default:
		case CHARACTER_ID_MAX:
			// If the character ID is invalid, generate no
			// animations by setting all of their lengths to zero.
			handler->define_stand_n.length = 0;
			handler->define_stand_s.length = 0;

			handler->define_taunt_n.length = 0;
			handler->define_taunt_s.length = 0;

			handler->define_hit_n.length = 0;
			handler->define_hit_s.length = 0;

			handler->define_ko_n.length = 0;
			handler->define_ko_s.length = 0;

			handler->define_run_n.length = 0;
			handler->define_run_s.length = 0;
			break;
	}

	// Now, using the defined animation data, generate the animations.
	
	handler->animations[CHAR_ANIM_IDLE]  = generateCharacterAnimation(
		handler, &handler->define_stand_n, &handler->define_stand_s);

	handler->animations[CHAR_ANIM_RUN]   = generateCharacterAnimation(
		handler, &handler->define_run_n, &handler->define_run_s);

	handler->animations[CHAR_ANIM_HIT]   = generateCharacterAnimation(
		handler, &handler->define_hit_n, &handler->define_hit_s);

	handler->animations[CHAR_ANIM_KO]   = generateCharacterAnimation(
		handler, &handler->define_ko_n, &handler->define_ko_s);

	handler->animations[CHAR_ANIM_TAUNT] = generateCharacterAnimation(
		handler, &handler->define_taunt_n, &handler->define_taunt_s);

	characterAnimationHandlers[id] = handler;

	return handler;
}

void freeCharacterAnimationHandler (enum AvatarId id) {
	CharacterAnimationHandler * handler = characterAnimationHandlers[id];

	if (handler == NULL)
		return;

	if (--handler->references < 1) {
		characterAnimationHandlers[id] = NULL;
		free(handler);
	}
}
