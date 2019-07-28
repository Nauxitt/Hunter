#include "stateengine.h"
#include "sprites.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

void getSpriteClip(SpriteSheet * sheet, int x, int y, SDL_Rect * dest){
	dest->x = x * sheet->src_w;
	dest->y = y * sheet->src_h;
	dest->w = sheet->src_w;
	dest->h = sheet->src_h;
}

void spritesheetBlit(SpriteSheet * sheet, int sx, int sy, int x, int y){
	SDL_Rect src;
	getSpriteClip(sheet, sx, sy, &src);
	SDL_Rect dest = {x, y, sheet->w, sheet->h};
	blit(sheet->texture, &src, &dest);
}

#define SHEET(name, path, w, h) .name = {NULL, path, w*2, h*2, w, h}

Textures textures = {
	SHEET(tiles         , "resources/hunter-tile.png", 32,16),
	SHEET(items         , "resources/items.png", 16,16),
	SHEET(daniel        , "resources/daniel.png", 0,0),
	SHEET(statbox       , "resources/statbox.png", 8,9),
	SHEET(cards         , "resources/cards.png", 16,18),
	SHEET(menu_gradient , "resources/menubar-gradient.png", 0,0),
	SHEET(menu_icons    , "resources/menu-battle-icons.png", 16,16),
	SHEET(crate         , "resources/crate.png", 0,0),
	SHEET(dice          , "resources/dice.png", 37, 48)
};


void loadSprites(){
	// Treat textures as an array of SpriteSheets and iterate across it, loading textures from their path properties.
	for(int x = sizeof(textures)/sizeof(SpriteSheet)-1; x >= 0; x--){
		SpriteSheet * sheet = ((SpriteSheet *) &textures) + x;
		sheet->texture = IMG_LoadTexture(game.renderer, sheet->path);
	}
}
