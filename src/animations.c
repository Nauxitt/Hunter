/*
   Definitions for sprite animations.
*/

#include "animations.h"

#define ANIMATION_NAME ANIM_HUNTER_STAND_S
ANIMATION {
	HUNTER_FRAME(0,   2,0, 750),
	HUNTER_FRAME_LAST(3,0, 750)
};
#undef ANIMATION_NAME

#define ANIMATION_NAME ANIM_HUNTER_STAND_N
ANIMATION {
	HUNTER_FRAME(0,   4,0, 750),
	HUNTER_FRAME_LAST(5,0, 750)
};
#undef ANIMATION_NAME

#define ANIM_HUNTER_RUN_FRAMELENGTH 75

#define ANIMATION_NAME ANIM_HUNTER_RUN_S
ANIMATION {
	HUNTER_FRAME(0,   0,1, ANIM_HUNTER_RUN_FRAMELENGTH),
	HUNTER_FRAME(1,   1,1, ANIM_HUNTER_RUN_FRAMELENGTH),
	HUNTER_FRAME(2,   2,1, ANIM_HUNTER_RUN_FRAMELENGTH),
	HUNTER_FRAME(3,   3,1, ANIM_HUNTER_RUN_FRAMELENGTH),
	HUNTER_FRAME_LAST(4,1, ANIM_HUNTER_RUN_FRAMELENGTH)
};
#undef ANIMATION_NAME

#define ANIMATION_NAME ANIM_HUNTER_RUN_N
ANIMATION {
	HUNTER_FRAME(0,   5,1, ANIM_HUNTER_RUN_FRAMELENGTH),
	HUNTER_FRAME(1,   6,1, ANIM_HUNTER_RUN_FRAMELENGTH),
	HUNTER_FRAME(2,   7,1, ANIM_HUNTER_RUN_FRAMELENGTH),
	HUNTER_FRAME(3,   8,1, ANIM_HUNTER_RUN_FRAMELENGTH),
	HUNTER_FRAME_LAST(9,1, ANIM_HUNTER_RUN_FRAMELENGTH)
};
#undef ANIMATION_NAME

#define ANIM_HUNTER_DAMAGE_FRAMELENGTH 100

#define ANIMATION_NAME ANIM_HUNTER_DAMAGE_S
ANIMATION {
	HUNTER_FRAME(0,   6,0, ANIM_HUNTER_DAMAGE_FRAMELENGTH),
	HUNTER_FRAME_LAST(7,0, ANIM_HUNTER_DAMAGE_FRAMELENGTH)
};
#undef ANIMATION_NAME

#define ANIMATION_NAME ANIM_HUNTER_DAMAGE_N
ANIMATION {
	HUNTER_FRAME(0,   8,0, ANIM_HUNTER_DAMAGE_FRAMELENGTH),
	HUNTER_FRAME_LAST(9,0, ANIM_HUNTER_DAMAGE_FRAMELENGTH)
};
#undef ANIMATION_NAME
