#ifndef __sprites_h
#define __sprites_h

#include <SDL2/SDL_image.h>

typedef struct _SpriteSheet {
	SDL_Texture * texture;
	char * path;
	int w, h;
	int src_w, src_h;
} SpriteSheet;

void loadSprites();

void getSpriteClip(SpriteSheet * sheet, int x, int y, SDL_Rect * dest);

typedef struct _Textures {
	SpriteSheet tiles;
	SpriteSheet items;
	SpriteSheet daniel;
	SpriteSheet statbox;
	SpriteSheet cards;
	SpriteSheet menu_gradient;
	SpriteSheet menu_icons;
	SpriteSheet crate;
} Textures;

extern Textures textures;

#endif
