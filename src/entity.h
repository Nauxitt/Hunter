#ifndef __entity_h
#define __entity_h

#include <SDL2/SDL.h>
#include <stdint.h>
#include "stateengine.h"

enum Direction {
	NORTH, EAST, SOUTH, WEST
};

typedef struct _AnimationFrame {
	SDL_Rect clip;
	int render_w, render_h;
	int center_x, center_y;
	uint8_t flip_h, flip_v;
	uint32_t duration;
	struct _AnimationFrame * next;
} AnimationFrame;


typedef struct _Entity {
	EventHandler handler;

	enum Direction direction;
	int x, y;
	int offset_x, offset_y;

	SDL_Texture * texture;
	AnimationFrame * animation;
	AnimationFrame * animation_frame;
	uint8_t animation_finished;
	uint8_t animation_loop;

	int flip_h, flip_v;
	uint32_t last_frame;
	float scale_w, scale_h;
} Entity;

#define Entity(E) ((Entity *) E)

void entityOnDraw(EventHandler * h);

/*
   Sets the Entity's animation, but only if the animation provided is different from the one already contained.  This allows the same animation to be set every frame without resetting the animation.
*/
void entitySetAnimation(Entity * entity, AnimationFrame * animation);
uint32_t animationGetDuration(AnimationFrame * animation);

#endif
