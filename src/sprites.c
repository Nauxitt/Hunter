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
	SHEET(tiles          , "resources/hunter-tile.png", 32,16),
	SHEET(items          , "resources/items.png", 16,16),
	SHEET(daniel         , "resources/daniel.png", 0,0),
	SHEET(statbox        , "resources/statbox.png", 8,9),
	SHEET(cards          , "resources/cards.png", 16,18),
	SHEET(menu_gradient  , "resources/menubar-gradient.png", 0,0),
	SHEET(menu_icons     , "resources/menu-battle-icons.png", 16,16),
	SHEET(crate          , "resources/crate.png", 0,0),
	SHEET(dice           , "resources/dice.png", 37, 48),
	SHEET(small_numbers  , "resources/small-numbers.png", 5, 6),
	SHEET(wallpapers     , "resources/wallpapers.png", 320, 208),
	SHEET(target_relic_panel, "resources/target-relic-panel.png", 0, 0)
};


void loadSprites(){
	// Treat textures as an array of SpriteSheets and iterate across it, loading textures from their path properties.
	for(int x = sizeof(textures)/sizeof(SpriteSheet)-1; x >= 0; x--){
		SpriteSheet * sheet = ((SpriteSheet *) &textures) + x;
		sheet->texture = IMG_LoadTexture(game.renderer, sheet->path);

		// Store sheet dimensions for convenience
		SDL_QueryTexture(
				sheet->texture,
				NULL, NULL,
				&sheet->sheet_w,
				&sheet->sheet_h
			);
		
		// If the sheet has unspecified dimensions, generate scaled size data.
		if(sheet->w == 0 || sheet->h == 0){
			sheet->src_w = sheet->sheet_w;
			sheet->src_h = sheet->sheet_h;

			sheet->w = sheet->src_w * 2;
			sheet->h = sheet->src_h * 2;
		}
		
		sheet->tiles_h = sheet->sheet_w / sheet->src_w;
		sheet->tiles_v = sheet->sheet_h / sheet->src_h;
		sheet->tiles_num = sheet->tiles_h * sheet->tiles_v;

		if(sheet->texture == NULL){
			printf("Could not load texture: %s\nSDL Image Error: %s\n", sheet->path, IMG_GetError());
		}
	}
}
