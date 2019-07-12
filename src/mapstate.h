/*
   This file contians a gamestate which handles the bulk of the game's features, specifically those of the screen which contains the map on which the game is played.  It contains code for rendering isometric map with entities an and an event queue system so that the game's turn-based behaviors do not find themselves in conflict.
*/

#ifndef __mapstate_h
#define __mapstate_h

#include "stateengine.h"
#include "hunter.h"
#include <stdint.h>
#include <stdio.h>

typedef struct _Entity Entity;

enum Direction {
	NORTH, EAST, SOUTH, WEST
};

#define TILE_LAYER_TILESELECTOR 0
#define TILE_LAYER_TRAPS 1
#define TILE_MAX_TRAPS 3
#define TILE_LAYER_CRATE 4
#define TILE_LAYER_HUNTER 5
#define TILE_LAYER_OVERHEADSELECTOR 6
#define TILE_ENTITY_LAYERS 7

enum WindowColor {
	WINDOW_BLUE,
	WINDOW_RED,
	WINDOW_ORANGE,
	WINDOW_GREEN
};


typedef struct _Tile {
	int val;        // 0 if void, otherwise floor
	int selected;

	int contents_len;
	Entity * contents[TILE_ENTITY_LAYERS];

	uint8_t tint_r, tint_g, tint_b;
	int x, y;
} Tile;


typedef struct _Map {
	Tile * tiles;
	int w, h;
} Map;


typedef struct _MenubarState {
	GameState state;
	SDL_Texture * background_texture;
	SDL_Texture * buttons_texture;
	MatchContext * match;
	uint8_t selector;  // Index of the item being selected
	uint8_t active;    // Whether the menubar is selecting anything
} MenubarState;


typedef struct _StatpanelState {
	GameState state;
} StatpanelState;


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
	struct _MapState * mapstate;

	enum Direction direction;
	int x, y;
	int offset_x, offset_y, offset_z;

	SDL_Texture * texture;
	AnimationFrame * animation;
	AnimationFrame * animation_frame;
	uint8_t animation_loop;
	int flip_h, flip_v;
	uint32_t last_frame;
	float scale_w, scale_h;
} Entity;


typedef struct _HunterEntity {
	Entity entity;
	Hunter * hunter;
} HunterEntity;


typedef struct _CrateEntity {
	Entity entity;
	Crate * crate;
} CrateEntity;


#define HUNTERS_COUNT 4


enum StatboxViews {
	STATBOX_VIEW_STATS,
	STATBOX_VIEW_ITEMS,
	STATBOX_VIEW_NONE
};


typedef struct _MapState {
	GameState state;
	MatchContext * match;

	int camera_x;
	int camera_y;

	SDL_Texture * statbox_texture;
	int statbox_tile_w, statbox_tile_h;
	int statbox_tile_src_w, statbox_tile_src_h;

	SDL_Texture * cards_texture;
	int card_w, card_h;
	int card_src_h, card_src_w;
	int card_selected;

	SDL_Texture * tiles_texture;
	int tile_w,     tile_h;
	int tile_src_w, tile_src_h;
	int tile_img_h;

	SDL_Texture * items_texture;
	int item_w, item_h;
	int item_src_w, item_src_h;

	SDL_Texture * daniel_texture;
	HunterEntity * daniel;
	HunterEntity hunters[HUNTERS_COUNT];

	int crates_len;
	CrateEntity * crates;

	enum StatboxViews statbox_view;
	
	Map * map;
	MenubarState * menubar;
	StatpanelState * statpanel;
} MapState;


/*
   Specialized ActionQeueue which with with containers a container for Entity data.
*/
typedef struct _ActionQueueEntity {
	ActionQueue action;
	MapState * state;
	Entity * entity;
	int target_x, target_y;
	int speed;
} ActionQueueEntity;

ActionQueueEntity * pushEntityAction(Entity * entity, char * type);
ActionQueueEntity * makeEntityAction(char * type);

// Convenience macros for casting to EventHandler subtype pointers
#define MapState(M) ((MapState *) M)
#define MenubarState(M) ((MenubarState *) M)
#define StatpanelState(S) ((StatepanelState *) S)
#define Entity(E) ((Entity *) E)
#define HunterEntity(E) ((HunterEntity *) E)
#define CrateEntity(E) ((CrateEntity *) E)

/*
   Macro which makes 2D for loops, where X and Y are names of integers that for_xy defines, and where W and H are the bounds iterated over.
*/
#define for_xy(X, Y, W, H) \
	for(int X=0; X<(W); X++) for(int Y=0; Y<(H); Y++)

MapState * makeMapState(MapState * mapstate, int map_w, int map_h);

Map * makeMap(int w, int h);
void freeMap(Map * map);
Tile * getTile(Map * map, int x, int y);

/*
   Translates map cell coordinates the pixel coordinates of that cell's center.
*/
int iso_x(MapState * state, int x, int y);
int iso_y(MapState * state, int x, int y);

/*
   Returns the Tile at a given pixel position, or returns NULL if the position is outside the map's boundries.
*/
Tile * getTileAtPx(MapState * state, float p_x, float p_y);

void entityOnDraw(EventHandler * h);

/*
   Sets the Entity's animation, but only if the animation provided is different from the one already contained.  This allows the same animation to be set every frame without resetting the animation.
*/
void entitySetAnimation(Entity * entity, AnimationFrame * animation);
void entitySetTile(Entity * e, int x, int y, int layer);

HunterEntity * initHunter(HunterEntity * hunter, MapState * state, SDL_Texture * texture);
CrateEntity * initCrateEntity(CrateEntity * hunter, MapState * state, SDL_Texture * texture);
void crateOnDraw(EventHandler * h);

void hunterSetTile(HunterEntity * e, int x, int y);
void crateSetTile(CrateEntity * c, int x, int y);

/*
   MapState event hooks
*/
void mapOnTick(EventHandler * h);
void mapOnKeyUp(EventHandler * h, SDL_Event * e);
void mapOnMouseDown(EventHandler * h, SDL_Event * e);
void mapOnDraw(EventHandler * h);

void mapMoveHunter(MapState * state, HunterEntity * hunter, int x, int y, int speed);
void mapOnTickMoveHunter(EventHandler * h);

void menuOnDraw(EventHandler * h);

void drawWindowPanel(MapState * state, enum WindowColor color, SDL_Rect * window_dest);
void drawStatbox(MapState * state, Hunter * hunter, enum StatboxViews view, enum WindowColor color, int x, int y);
void drawStatboxItems(MapState * state, Hunter * hunter, int x, int y);
void drawStatboxStats(MapState * state, Hunter * hunter, int x, int y);

#endif
