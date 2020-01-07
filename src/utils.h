#ifndef __utils_h
#define __utils_h

#include <SDL2/SDL.h>

void printRect(SDL_Rect * rect);

/*
   Macro which makes 2D for loops, where X and Y are names of integers that for_xy defines, and where W and H are the bounds iterated over.
*/
#define for_xy(X, Y, W, H) \
	for(int X=0; X<(W); X++) for(int Y=0; Y<(H); Y++)

#define XY(ptr) (ptr)->x, (ptr)->y

#define ADJACENT_MAP(OP, X, Y) \
	OP(X  , Y-1); \
	OP(X+1, Y  ); \
	OP(X  , Y+1); \
	OP(X-1, Y  );

#endif
