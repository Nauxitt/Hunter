#ifndef __character_h
#define __character_h

#include "entity.h"

typedef struct _HunterEntity HunterEntity;

enum AvatarId {
	CHARACTER_DANIEL,
	CHARACTER_CHRISTINA,
	CHARACTER_SIMON,
	CHARACTER_ID_MAX
};

enum CharacterAnimationType {
	CHAR_ANIM_IDLE = 0,  // Default fallback animation
	CHAR_ANIM_HIT,
	CHAR_ANIM_TAUNT,
	CHAR_ANIM_KO,
	CHAR_ANIM_RUN,
	CHAR_ANIM_MAX
};

typedef struct _CharacterAnimation {
	// Which spritesheet-specific animation to use for a given N/S direction. If
	// only one is set, it will be used.
	AnimationFrame * entity_animation_n;
	AnimationFrame * entity_animation_s;

	// The next animation, or NULL to terminate or loop, depending on external
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

typedef struct {
	uint8_t row, col, length;
	uint32_t duration;
} CharacterAnimationSlice;

typedef struct _CharacterAnimationHandler {
	int id;
	char * name;

	// Track the number of references to this handler, so that it doesn't
	// become free'd prematurely.
	int references;

	SDL_Texture * texture;
	CharacterAnimation * animations[CHAR_ANIM_MAX];

	/* 
	 * Grid-based animation semiautomatic-generator
	 * 
	 * The following fields can be set to programmatically generate character
	 * animations based on horizontal stretches of tiles within a sprite sheet.
	 * Attack animations are the exception, needing to be defined elsewhere due
	 * to being more complex.
	 */
	int scale_w, scale_h;
	int tile_w, tile_h;
	int center_x, center_y;
	
	CharacterAnimationSlice define_stand_n;
	CharacterAnimationSlice define_stand_s;

	CharacterAnimationSlice define_taunt_n;
	CharacterAnimationSlice define_taunt_s;

	CharacterAnimationSlice define_hit_n;
	CharacterAnimationSlice define_hit_s;

	CharacterAnimationSlice define_ko_n;
	CharacterAnimationSlice define_ko_s;

	CharacterAnimationSlice define_run_n;
	CharacterAnimationSlice define_run_s;
} CharacterAnimationHandler;

typedef struct _CharacterAnimationContext {
	CharacterAnimationHandler * handler;
	CharacterAnimation * animation_start;
	CharacterAnimation * animation_current;
	HunterEntity * character;

	enum Direction direction;
	enum Direction last_direction;  // Used for tracking directional changes

	uint8_t finished;
	uint8_t loop;
} CharacterAnimationContext;

GameState characterBlockState;

void characterAnimationBlockTicks(HunterEntity * entity);
void characterAnimationBlockTicksStateOnTick(EventHandler * h);

void initCharacterAnimations();
CharacterAnimationHandler * getCharacterAnimationHandler(enum AvatarId id);
void freeCharacterAnimationHandler (enum AvatarId id);

void animationContextSetCharacterAnimation(CharacterAnimationContext * a_context, CharacterAnimation * animation);
void characterEntityOnDraw(EventHandler * h);

void animationContextSetCharacterAnimation(CharacterAnimationContext * a_context, CharacterAnimation * c_animation);

CharacterAnimationHandler * getCharacterAnimationHandler (enum AvatarId id);

AnimationFrame * generateCharacterAnimationFrames (CharacterAnimationHandler * handler, CharacterAnimationSlice * slice);
CharacterAnimation * generateCharacterAnimation (CharacterAnimationHandler * handler, CharacterAnimationSlice * slice_n, CharacterAnimationSlice * slice_s);

void characterLoopAnimation (HunterEntity * character, enum CharacterAnimationType animation_id);
void characterSetAnimation (HunterEntity * character, enum CharacterAnimationType animation_id);

int animationHandlerGetAnimationDuration (CharacterAnimationHandler * handler, enum CharacterAnimationType id);

CharacterAnimationContext * initCharacterAnimationContext(CharacterAnimationContext * a_context, CharacterAnimationHandler * handler, HunterEntity * character);


extern CharacterAnimationHandler * characterAnimationHandlers[CHARACTER_ID_MAX];

#endif
