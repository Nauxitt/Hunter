/*
   draw.h

   Contains functions used for drawing common game elements.
*/

#ifndef __draw_h
#define __draw_h

#include "hunter.h"
#include <SDL2/SDL.h>

enum WindowColor {
	WINDOW_BLUE,
	WINDOW_RED,
	WINDOW_ORANGE,
	WINDOW_GREEN
};

enum StatboxViews {
	STATBOX_VIEW_STATS,
	STATBOX_VIEW_ITEMS,
	STATBOX_VIEW_NONE
};

void drawWindowPanel(enum WindowColor color, SDL_Rect * window_dest);
void drawSmallNumber(int x, int y, int n);
void drawBigNumber(int x, int y, int n);
void drawBigRedNumber(int x, int y, int n);
void drawCard(int x, int y, Card * card);
void drawStatbox(Hunter * hunter, enum StatboxViews view, enum WindowColor color, int x, int y);
void drawStatboxItems(Hunter * hunter, int x, int y);
void drawRelic(Relic * relic, int x, int y);
void drawStatboxStats(Hunter * hunter, int x, int y);

void drawDiceBack(SDL_Rect * dest);
void drawDamageDice(int num, int x, int y);
void drawMoveDice(int num, int x, int y);
void drawDefenseDice(int num, int x, int y);

void drawDeckIndicator(int x, int y, int len);
void drawWallpaper(int id);
void drawWallpaperTransition(int id, int gap);

void drawChar(char c, int x, int y);
void drawString(char * str, int x, int y);

#endif
