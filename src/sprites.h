#ifndef __sprites_h
#define __sprites_h

#include <SDL2/SDL_image.h>

typedef struct _SpriteSheet {
	SDL_Texture * texture;
	char * path;
	int w, h;
	int src_w, src_h;
	int sheet_w, sheet_h;
	int tiles_h, tiles_v, tiles_num;
} SpriteSheet;

void loadSprites();

void getSpriteClip(SpriteSheet * sheet, int x, int y, SDL_Rect * dest);
void spritesheetBlit(SpriteSheet * sheet, int sx, int sy, int x, int y);

typedef struct _Textures {
	SpriteSheet tiles;
	SpriteSheet items;
	SpriteSheet daniel;
	SpriteSheet statbox;
	SpriteSheet cards;
	SpriteSheet menu_gradient;
	SpriteSheet menu_icons;
	SpriteSheet crate;
	SpriteSheet dice;
	SpriteSheet font;
	SpriteSheet small_numbers;
	SpriteSheet wallpapers;
	SpriteSheet bubble;
	SpriteSheet character_portraits;
	SpriteSheet target_relic_panel;
} Textures;

extern Textures textures;

#endif
