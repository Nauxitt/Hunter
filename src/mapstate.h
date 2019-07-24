/*
   This file contians a gamestate which handles the bulk of the game's features, specifically those of the screen which contains the map on which the game is played.  It contains code for rendering isometric map with entities an and an event queue system so that the game's turn-based behaviors do not find themselves in conflict.
*/

#ifndef __mapstate_h
#define __mapstate_h

#include "stateengine.h"
#include "hunter.h"
#include <stdint.h>
#include <stdio.h>
#include "entity.h"
#include "menubar.h"
#include "draw.h"

typedef struct _Entity Entity;

#define TILE_LAYER_TILESELECTOR 0
#define TILE_LAYER_TRAPS 1
#define TILE_MAX_TRAPS 3
#define TILE_LAYER_CRATE 4
#define TILE_LAYER_HUNTER 5
#define TILE_LAYER_OVERHEADSELECTOR 6
#define TILE_ENTITY_LAYERS 7

typedef struct _TileEntity {
	Entity entity;
	struct _MapState * mapstate;
	int x, y;
	int offset_x, offset_y, offset_z;
} TileEntity;

typedef struct _MapStateTile {
	int val;        // 0 if void, otherwise floor
	int selected;

	int contents_len;
	TileEntity * contents[TILE_ENTITY_LAYERS];

	uint8_t tint_r, tint_g, tint_b;
	int x, y;
} MapStateTile;


typedef struct _MapStateMap {
	MapStateTile * tiles;
	int w, h;
} MapStateMap;


typedef struct _StatpanelState {
	GameState state;
} StatpanelState;

typedef struct _HunterEntity {
	TileEntity entity;
	Hunter * hunter;
} HunterEntity;


typedef struct _CrateEntity {
	TileEntity entity;
	Crate * crate;
} CrateEntity;


#define HUNTERS_COUNT 4


typedef struct _MapState {
	GameState state;
	MatchContext * match;

	int camera_x;
	int camera_y;

	int card_selected;

	int crates_len;
	CrateEntity * crates;

	enum StatboxViews statbox_view;
	
	MapStateMap * map;
	MenubarState * menubar;
	StatpanelState * statpanel;

	SDL_Texture * tiles_texture;
	int tile_w,     tile_h;
	int tile_src_w, tile_src_h;
	int tile_img_h;

	HunterEntity * daniel;
	HunterEntity hunters[HUNTERS_COUNT];
} MapState;


/*
   Specialized ActionQeueue which with with containers a container for Entity data.
*/
typedef struct _ActionQueueEntity {
	ActionQueue action;
	MapState * state;
	Entity * entity;
	Relic * relic;
	int target_x, target_y;
	int speed;
} ActionQueueEntity;

ActionQueueEntity * pushEntityAction(Entity * entity, char * type);
ActionQueueEntity * makeEntityAction(char * type);

// Convenience macros for casting to EventHandler subtype pointers
#define MapState(S) ((MapState *) S)
#define StatpanelState(S) ((StatepanelState *) S)
#define TileEntity(E) ((TileEntity *) E)
#define HunterEntity(E) ((HunterEntity *) E)
#define CrateEntity(E) ((CrateEntity *) E)

MapState * makeMapState(MapState * mapstate, MatchContext * match);

MapStateMap * makeMap(int w, int h);
void freeMap(MapStateMap * map);
MapStateTile * getTile(MapStateMap * map, int x, int y);

/*
   Translates map cell coordinates the pixel coordinates of that cell's center.
*/
int iso_x(MapState * state, int x, int y);
int iso_y(MapState * state, int x, int y);

/*
   Returns the MapStateTile at a given pixel position, or returns NULL if the position is outside the map's boundries.
*/
MapStateTile * getTileAtPx(MapState * state, float p_x, float p_y);

void tileEntityOnDraw(EventHandler * h);
HunterEntity * initHunterEntity(HunterEntity * hunter, MapState * state, SDL_Texture * texture);
CrateEntity * initCrateEntity(CrateEntity * hunter, MapState * state, SDL_Texture * texture);
void crateOnDraw(EventHandler * h);

void tileEntitySetTile(TileEntity * e, int x, int y, int layer);
void hunterSetTile(HunterEntity * h, int x, int y);
void crateSetTile(CrateEntity * c, int x, int y);


/*
   MapState event hooks
*/
void mapOnTick(EventHandler * h);
void mapOnKeyUp(EventHandler * h, SDL_Event * e);
void mapOnMouseDown(EventHandler * h, SDL_Event * e);
void mapOnDraw(EventHandler * h);

void mapGiveRelic(HunterEntity * hunter, Relic * relic);
void mapOnDrawGiveRelic(EventHandler * h);
void mapEnterCombat(MapState * state);

void mapMoveHunter(MapState * state, HunterEntity * hunter, int x, int y, int speed);
void mapOnTickMoveHunter(EventHandler * h);

#endif
