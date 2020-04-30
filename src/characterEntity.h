#ifndef __characterEntity_h
#define __characterEntity_h

#include "entity.h"
#include "mapstate.h"

enum CharacterAnimationType {
	CHAR_ANIM_IDLE = 0,  // Default fallback animation
	CHAR_ANIM_RUN,
	CHAR_ANIM_HIT,
	CHAR_ANIM_TAUNT,
	CHAR_ANIM_MAX
};

typedef struct _CharacterAnimation {
	// Which spritesheet-specific animation to use for a given N/S direction. If
	// only one is set, it will be used.
	AnimationFrame * entity_animation_n;
	AnimationFrame * entity_animation_s;

	// The next animation, or NULL to terminate or loop, depending on extermal
	// implementation
	struct _CharacterAnimation * next;

	/* The redirect property causes the animation handler to select another
	 * animation type, but only after checking that no other animation
	 * properties in this struct are set.  This allows one animation to ne used
	 * for multiple animation types.
	 */
	enum CharacterAnimationType redirect;

	/* Combat-related properies */
	int is_combat_animation;
	int jump_height;
	int target_run_distance_from;

	// In combat, if an entity is specified as a projectile, animate it.  This
	// animation is ended when the projectile is despawned.
	Entity * projectile;
	int projectile_speed;
	
	// A sprite to generate in combat over the attacker, if set;
	Entity * particle;
} CharacterAnimation;

typedef struct _CharacterEntityAnimationHandler {
	int avatar_id;
	SDL_Texture * spritesheet;
	CharacterAnimation * animations[CHAR_ANIM_MAX];
} CharacterEntityAnimationHandler;

typedef struct _CharacterEntityAnimationContext {
	CharacterEntityAnimationHandler * handler;
	CharacterAnimation * animation;
	HunterEntity * entity;
	uint8_t finished;
} CharacterEntityAnimationContext;

GameState characterEntityBlockState;

void characterEntityAnimationBlockTicks(HunterEntity * entity);
void characterEntityAnimationBlockTicksStateOnTick(EventHandler * h);

#endif
