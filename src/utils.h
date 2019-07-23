#ifndef __utils_h
#define __utils_h

void printRect(SDL_Rect * rect);

/*
   Macro which makes 2D for loops, where X and Y are names of integers that for_xy defines, and where W and H are the bounds iterated over.
*/
#define for_xy(X, Y, W, H) \
	for(int X=0; X<(W); X++) for(int Y=0; Y<(H); Y++)

#endif
