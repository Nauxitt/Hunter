/*
   This file contains macros for making sprite animations, as well as declarations for sprite animations.
*/

#ifndef __animations_h
#define __animations_h

#include "mapstate.h"

#define ANIMATION AnimationFrame ANIMATION_NAME[] =

#define SPRITE_HUNTER_W 32
#define SPRITE_HUNTER_H 40

#define HUNTER_FRAME_N(n, x, y, d) \
	{ \
		.clip = {SPRITE_HUNTER_W * x, SPRITE_HUNTER_H * y, SPRITE_HUNTER_W, SPRITE_HUNTER_H}, \
		.center_x = SPRITE_HUNTER_W/2, \
		.center_y = SPRITE_HUNTER_H-6, \
		.duration = d, \
		.next = n \
	}

#define HUNTER_FRAME(n, x, y, d) \
	HUNTER_FRAME_N(&ANIMATION_NAME[n+1], x, y, d)

#define HUNTER_FRAME_LAST(x, y, d) \
	HUNTER_FRAME_N(NULL, x, y, d)

extern AnimationFrame ANIM_HUNTER_STAND_S[];
extern AnimationFrame ANIM_HUNTER_STAND_N[];
extern AnimationFrame ANIM_HUNTER_RUN_S[];
extern AnimationFrame ANIM_HUNTER_RUN_N[];
extern AnimationFrame ANIM_HUNTER_DAMAGE_N[];
extern AnimationFrame ANIM_HUNTER_DAMAGE_S[];

#endif
