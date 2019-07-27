#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "stateengine.h"
#include "mapstate.h"
#include "animations.h"
#include "sprites.h"
#include "draw.h"
#include "entity.h"
#include "menubar.h"
#include "utils.h"
#include "handstate.h"

extern Game game;
extern MapState mapstate;
extern MenubarState menubar;
extern StatpanelState statpanel;

int iso_x(MapState * state, int x, int y){
	return state->camera_x + (x-y) * state->tile_w/2 + state->tile_w/2;
};

int iso_y(MapState * state, int x, int y){
	return state->camera_y + (x+y) * state->tile_h/2 + state->tile_h/2;
};

MapStateMap * makeMap(int w, int h){
	MapStateMap * ret = (MapStateMap*) malloc(sizeof(MapStateMap));
	ret->w = w;
	ret->h = h;
	ret->tiles = (MapStateTile*) calloc(sizeof(MapStateTile), w*h);

	for(int n=0; n < w*h; n++){
		ret->tiles[n].x = n % w;
		ret->tiles[n].y = n / w;
		ret->tiles[n].tint_r = 255;
		ret->tiles[n].tint_g = 255;
		ret->tiles[n].tint_b = 255;
	}

	return ret;
}

MapState * makeMapState(MapState * mapstate, MatchContext * match){
	if(mapstate == NULL)
		mapstate = MapState(calloc(sizeof(MapState), 1));

	mapstate->menubar = initMenu(NULL, match);
	mapstate->match = match;
	
	// TODO: move menubar initialization into menubar.c
	mapstate->map = makeMap(match->map_w, match->map_h);

	mapstate->tile_w = 64;
	mapstate->tile_h = 32;
	mapstate->tile_src_w = 32;
	mapstate->tile_src_h = 16;
	mapstate->tile_img_h = 32;

	// TODO: calculate center
	mapstate->camera_x = 280;
	mapstate->camera_y = 125;

	// Load some textures
	mapstate->tiles_texture = textures.tiles.texture;

	EventHandler(mapstate)->type = "MapState";
	EventHandler(mapstate)->onTick = mapOnTick;
	EventHandler(mapstate)->onMouseDown = mapOnMouseDown;
	EventHandler(mapstate)->onDraw = mapOnDraw;
	EventHandler(mapstate)->onKeyUp = mapOnKeyUp;

	pushAction("poll_turn_action");
	return mapstate;
}

void mapSetSelection(MapStateMap * map, int value){
	for(int n=0; n < map->w*map->h; n++)
		map->tiles[n].selected = value;
}

void mapSelectAll(MapStateMap * map){  mapSetSelection(map, 1); }
void mapSelectNone(MapStateMap * map){ mapSetSelection(map, 0); }
void mapSelectRange(MapStateMap * map, int c_x, int c_y, int range){
	for_xy(x, y, map->w, map->h){
		MapStateTile * tile = getTile(map, x,y);
		tile->selected = (abs(c_x - x) + abs(c_y - y)) <= range;
	}
}

void freeMap(MapStateMap * map){
	free(map->tiles);
	free(map);
}

MapStateTile * getTile(MapStateMap * map, int x, int y){
	return &(map->tiles[y * map->w + x]);
}

MapStateTile * getTileAtPx(MapState * state, float p_x, float p_y){
	// Account for the tile's diagonal edges
	p_x -= state->tile_w / 4;
	p_y -= state->tile_h / 4;

	// MapStateTile cell position
	int tx = p_x/state->tile_w + p_y/state->tile_h;
	int ty = p_y/state->tile_h - p_x/state->tile_w;

	
	// Return NULL if pixel position is outside of map boundaries
	if((tx < 0) | (tx > state->map->w)) return NULL;
	if((ty < 0) | (ty > state->map->h)) return NULL;

	return getTile(state->map, tx, ty);
}

void hunterSetTile(HunterEntity * e, int x, int y){
	tileEntitySetTile(TileEntity(e), x, y, TILE_LAYER_HUNTER);
	e->hunter->x = x;
	e->hunter->y = y;
}

void crateSetTile(CrateEntity * c, int x, int y){
	tileEntitySetTile(TileEntity(c), x, y, TILE_LAYER_CRATE);
	c->crate->x = x;
	c->crate->y = y;
}

HunterEntity * initHunterEntity(HunterEntity * hunter, MapState * state, SDL_Texture * texture){
	if(hunter == NULL)
		hunter = HunterEntity(malloc(sizeof(HunterEntity)));
	
	Entity * e = Entity(hunter);
	TileEntity * te = TileEntity(hunter);

	te->mapstate = state;
	e->texture = texture;
	e->animation = (AnimationFrame*) &ANIM_HUNTER_STAND_S;
	e->animation_loop = 1;
	e->scale_w = 2;
	e->scale_h = 2;
	EventHandler(hunter)->type = "HunterEntity";
	EventHandler(hunter)->onDraw = tileEntityOnDraw;
	return hunter;
}

CrateEntity * initCrateEntity(CrateEntity * crate, MapState * state, SDL_Texture * texture){
	if(crate == NULL)
		crate = CrateEntity(calloc(sizeof(CrateEntity), 1));

	Entity * e = Entity(crate);
	TileEntity * te = TileEntity(crate);
	te->mapstate = state;
	e->texture = texture;
	e->animation = NULL;
	e->scale_w = 2;
	e->scale_h = 2;
	te->offset_z = state->tile_h / 2;

	EventHandler(crate)->type = "CrateEntity";
	EventHandler(crate)->onDraw = crateOnDraw;
	return crate;
}

void crateOnDraw(EventHandler * h){
	if(CrateEntity(h)->crate->exists)
		tileEntityOnDraw(h);
}


void mapOnTick(EventHandler * h){
	MapState * state = MapState(h);
	MatchContext * match = state->match;
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	Hunter * active_player = match->characters[match->active_player];

	HunterEntity * active_hunter_entity = NULL;
	for(int n=0; n < HUNTERS_COUNT; n++){
		active_hunter_entity = state->hunters+n;
		if(active_hunter_entity->hunter == active_player)
			break;
	}

	state->menubar->match = state->match;

	int breaker = 1;
	while((breaker == 1) && (match->action)){
		MatchAction * action = match->action;
		// printf("[%d] %s: 0x%x\n", breaker, getMatchActionName(action->type), (int) action);

		HunterEntity * action_hunter_entity = NULL;
		for(int n=0; n < HUNTERS_COUNT; n++){
			action_hunter_entity = state->hunters+n;
			if(action_hunter_entity->hunter == action->actor)
				break;
		}

		switch(action->type){
			case POLL_TURN_ACTION:
			case POLL_MOVE_CARD_ACTION:
			case POLL_MOVE_ACTION:
			case POLL_ATTACK_ACTION:
			case POLL_COMBAT_CARD_ACTION:
			case POLL_COMBAT_ACTION:
				breaker = 0;

			case BEGIN_MATCH_ACTION:
			case TURN_START_ACTION:
			case TURN_END_ACTION:
			case DRAW_CARD_ACTION:
			case HEAL_ACTION:
			case MOVE_ACTION:
			case ATTACK_ACTION:
			case REST_ACTION:
			case COUNTERATTACK_ACTION:
			case DEFEND_ACTION:
			case ESCAPE_ACTION:
			case SURRENDER_ACTION:
			case USE_CARD_ACTION:
			case ROLL_MOVE_DICE_ACTION:
			case ROLL_ATTACK_DICE_ACTION:
			case ROLL_DEFENSE_DICE_ACTION:
				matchCycle(match);
				break;

			case OPEN_CRATE_ACTION:
				matchCycle(match);
				for_xy(x, y, state->map->w, state->map->h){
					MapStateTile * tile = getTile(state->map, x, y);
					CrateEntity * entity = CrateEntity(tile->contents[TILE_LAYER_CRATE]);

					if(!entity || (entity->crate != action->crate))
						continue;

					tile->contents[TILE_LAYER_CRATE] = NULL;
					break;
				}
				break;

			case GIVE_RELIC_ACTION:
				mapGiveRelic(action_hunter_entity, action->relic);
				matchCycle(match);
				return;

			case MOVE_STEP_ACTION:
				mapMoveHunter(
						state, action_hunter_entity,
						action->x, action->y, 6
					);
				onTick(EventHandler(game.state));
				return;

			case END_MOVE_ACTION:
				switch(Entity(action_hunter_entity)->direction){
					case NORTH:
					case WEST:
						entitySetAnimation(
								Entity(action_hunter_entity),
								(AnimationFrame *) &ANIM_HUNTER_STAND_N
							);
						break;

					case SOUTH:
					case EAST:
						entitySetAnimation(
								Entity(action_hunter_entity),
								(AnimationFrame *) &ANIM_HUNTER_STAND_S
							);
						break;
				}
				breaker = 0;
				matchCycle(match);
				break;

			default:
				breaker = 0;
				break;
		}
	}

	MatchAction * action = match->action;

	if(action->type == POLL_MOVE_ACTION){
		// Pan camera with arrow keys
		if(keys[SDL_SCANCODE_UP])    state->camera_y += 10;
		if(keys[SDL_SCANCODE_DOWN])  state->camera_y -= 10;
		if(keys[SDL_SCANCODE_LEFT])  state->camera_x += 10;
		if(keys[SDL_SCANCODE_RIGHT]) state->camera_x -= 10;

		Statset * stats = hunterStats(active_player);
		mapSelectRange(
				state->map,
				active_player->x,
				active_player->y,
				stats->mov + 1
			);
	}

	// Select card
	else if(pollAction("poll_move_card_select")){
		if(state->card_selected != NULL)
			postTurnAction(match, MOVE_ACTION, NULL, state->card_selected);
		nextAction();
	}

	EventHandler * menu_handler = EventHandler(state->menubar);
	if(menu_handler && menu_handler->onTick)
		menu_handler->onTick(menu_handler);
}

void mapMoveHunter(MapState * state, HunterEntity * hunter, int x, int y, int speed){
	/*
	   Pushes a new GameState on the stack, which redirects its draw events the the previous (MapState) state.  The new state's onTick event directs the hunter to the given target position, then pops the state upon completion.
	*/

	GameState * moveState = makeGameState();
	gamePushState(moveState);

	ActionQueueEntity * action = makeEntityAction("move_entity");
	action->entity = Entity(hunter);
	action->target_x = x;
	action->target_y = y;
	action->speed = 6;

	EventHandler(moveState)->type = "MapState.mapMoveHunter";
	EventHandler(moveState)->data = action;
	EventHandler(moveState)->onDraw = prevStateOnDraw;
	EventHandler(moveState)->onTick = mapOnTickMoveHunter;
}

void mapOnTickMoveHunter(EventHandler * h){
	/*
		Animate moving
		If done, pop gamestate, perform a match cycle, and then fire the previous state's onTick event
	*/

	MapState * state = MapState(GameState(h)->prevState);
	ActionQueueEntity * action = (ActionQueueEntity*) h->data;

	int target_x = action->target_x;
	int target_y = action->target_y;
	int speed = action->speed;
	TileEntity * entity = TileEntity(action->entity);

	// Move TileEntity offset and cell position
	// Set running animation
	
	if(entity->x > target_x){
		Entity(entity)->direction = WEST; Entity(entity)->flip_h = 1;

		entity->offset_x -= speed;
		if(entity->offset_x <= -state->tile_w/2)
			entity->offset_x = 0, entity->x--;
	}
	else if(entity->x < target_x){
		Entity(entity)->direction = EAST; Entity(entity)->flip_h = 1;

		entity->offset_x += speed;
		if(entity->offset_x >= state->tile_w/2)
			entity->offset_x = 0, entity->x++;
	}
	else if(entity->y > target_y){
		Entity(entity)->direction = NORTH; Entity(entity)->flip_h = 0;

		entity->offset_y -= speed;
		if(entity->offset_y <= -state->tile_h)
			entity->offset_y = 0, entity->y--;
	}
	else if(entity->y < target_y){
		Entity(entity)->direction = SOUTH; Entity(entity)->flip_h = 0;

		entity->offset_y += speed;
		if(entity->offset_y >= state->tile_h)
			entity->offset_y = 0, entity->y++;
	}

	switch(Entity(entity)->direction){
		case NORTH:
		case WEST:
			entitySetAnimation(
					Entity(entity), (AnimationFrame *) &ANIM_HUNTER_RUN_N
				);
			break;

		case SOUTH:
		case EAST:
			entitySetAnimation(
					Entity(entity), (AnimationFrame *) &ANIM_HUNTER_RUN_S
				);
			break;
	}

	hunterSetTile(HunterEntity(entity), entity->x, entity->y);
	
	// If finished running, stop.  Switch to standing animation
	if(
		(entity->x == target_x) &&
		(entity->y == target_y)
	){

		// Exit this state and do cleanup
		gamePopState();
		// onTick(EventHandler(state));
		free(action);
		free(h);

		// Runs the move game action, updating the game logic with the completed movement
		matchCycle(MapState(game.state)->match);
	}
}

void mapGiveRelic(HunterEntity * hunter, Relic * relic){
	GameState * giveState = makeGameState();
	gamePushState(giveState);

	ActionQueueEntity * action = makeEntityAction("give_item");
	action->relic = relic;
	action->entity = Entity(hunter);

	EventHandler(giveState)->type = "MapState.mapGiveRelic";
	EventHandler(giveState)->data = action;
	EventHandler(giveState)->onDraw = mapOnDrawGiveRelic;
}

void mapOnDrawGiveRelic(EventHandler * h){
	GameState * giveState = GameState(h);
	MapState * mapstate = MapState(giveState->prevState);
	ActionQueueEntity * action = EventHandler(giveState)->data;

	uint32_t duration = giveState->duration;
	Relic * relic = action->relic;
	HunterEntity * hunter = HunterEntity(action->entity);
	
	float drop_speed = 1.25;
	int pause_duration = 120;
	int x = iso_x(mapstate, TileEntity(hunter)->x, TileEntity(hunter)->y) - textures.items.w/2;
	int y = duration * drop_speed;
	int end_y = iso_y(mapstate, TileEntity(hunter)->x, TileEntity(hunter)->y) - 64 - textures.items.h;

	if(y > end_y)
		y = end_y;

	onDraw(EventHandler(mapstate));

	SDL_Rect clip = {0, 64, game.w, game.h-64};
	SDL_RenderSetClipRect(game.renderer, &clip);

	drawRelic(relic, x, y);

	if(duration >= duration/drop_speed + pause_duration){
		gamePopState();
		matchCycle(mapstate->match);
		free(action);
		free(giveState);
	}

	SDL_RenderSetClipRect(game.renderer, NULL);
}

void mapOnKeyUp(EventHandler * h, SDL_Event * e){
	MapState * state = MapState(h);
	MatchContext * match = state->match;
	MatchAction * action = match->action;
	Hunter * active_player = match->characters[match->active_player];

	if(e->key.keysym.scancode == SDL_SCANCODE_TAB){
		state->statbox_view++;
		if(state->statbox_view == STATBOX_VIEW_NONE + 1)
			state->statbox_view = 0;
	}

	// Menubar arrow keys
	if(action->type == POLL_TURN_ACTION){
		if(pollAction("poll_turn_action")){
			switch(e->key.keysym.scancode){
				case SDL_SCANCODE_LEFT:
				case SDL_SCANCODE_RIGHT:
					onKeyUp(state->menubar, e);
					break;

				case SDL_SCANCODE_SPACE:

					// Move action
					if(state->menubar->selector == 0){
						HandState * handState = makeHandState(NULL, active_player, 32, 72);

						handState->card_target = &state->card_selected;

						pushAction("poll_move_card_select");
						gamePushState((GameState*) handState);
					}

					// Combat Action
					else if (state->menubar->selector == 1){
						// TODO: enter attack target selection
					}

					// Rest Action
					else if(state->menubar->selector == 2){
						postTurnAction(match, REST_ACTION, NULL, NULL);
					}

					state->menubar->selector = 0;
					break;

				default:
					break;
			}
		}
	}

	else if(pollAction("poll_tile_select")){
		if(e->key.keysym.scancode == SDL_SCANCODE_ESCAPE)
			nextAction(state);
	}
}

void mapOnMouseDown(EventHandler * h, SDL_Event * e){
	MapState * state = MapState(h);
	MatchContext * match = state->match;
	SDL_MouseButtonEvent me = e->button;

	MapStateTile * t = getTileAtPx(
			state,
			me.x - state->camera_x,
			me.y - state->camera_y
		);

	if(t){
		if(match->action->type == POLL_MOVE_ACTION){
			if((t->selected) && (t->val)){
				postMoveAction(
						match, NULL, t->x, t->y
					);
				mapSelectNone(state->map);
			}
		}
	}
}

void mapOnDraw(EventHandler * h){
	MapState * state = MapState(h);
	MapStateMap * map = state->map;
	
	SDL_Rect srcrect  = {0, 0, state->tile_src_w, state->tile_img_h};

	SDL_Rect destrect = {
			0, 0,
			state->tile_w,
			state->tile_h * state->tile_img_h / state->tile_src_h
		};
	

	// Draw tiles
	for_xy(x, y, map->w, map->h){
		MapStateTile * tile = getTile(map, x, y);
		
		if(!tile->val)
			continue;

		destrect.x = iso_x(state, x, y) - state->tile_w/2;
		destrect.y = iso_y(state, x, y) - state->tile_h/2;

		if(tile->selected){
			tile->tint_r = 128; tile->tint_g = 128; tile->tint_b = 255;
		}
		else {
			tile->tint_r = 255; tile->tint_g = 255; tile->tint_b = 255;
		}
		
		SDL_SetTextureColorMod(
				state->tiles_texture,
				tile->tint_r, tile->tint_g, tile->tint_b
			);
		
		blit(state->tiles_texture, &srcrect, &destrect);
	}
	
	// Iterate through tiles and draw entities
	for_xy(x, y, map->w, map->h){
		MapStateTile * tile = getTile(map, x, y);

		for(int e=0; e < TILE_ENTITY_LAYERS; e++){
			TileEntity * entity = tile->contents[e];

			if(entity == NULL)
				continue;

			if(EventHandler(entity))
				EventHandler(entity)->onDraw(EventHandler(entity));
		}
	}

	// TODO: draw screen tint
	
	// Forward event to menubar
	EventHandler * menu_handler = EventHandler(state->menubar);
	if(menu_handler && menu_handler->onDraw)
		menu_handler->onDraw(menu_handler);

	// Draw stat windows, character stats
	int panel_gutter = 4;
	int panel_w = (game.w - 16*2 - 4*3) / 4;
	for(int h=0; h < HUNTERS_COUNT; h++){
		Hunter * hunter = state->hunters[h].hunter;

		drawStatbox(
				hunter,
				(enum StatboxViews) state->statbox_view,
				(enum WindowColor) h,
				16 + (panel_w+panel_gutter)*h,
				game.h-160-panel_gutter
			);
	}
}

ActionQueueEntity * makeEntityAction(char * type){
	ActionQueueEntity * ret = (ActionQueueEntity *) malloc(sizeof(ActionQueueEntity));
	ActionQueue(ret)->type = type;
	return ret;
}

ActionQueueEntity * pushEntityAction(Entity * entity, char * type){
	ActionQueueEntity * new_action = makeEntityAction(type);
	ActionQueue(new_action)->next = game.action;
	ActionQueue(new_action)->start = SDL_GetTicks();
	new_action->entity = entity;
	game.action = ActionQueue(new_action);
	return new_action;
}

void tileEntityOnDraw(EventHandler * h){
	/*
	   Uses TileEntity isometric coordinates and reference to the MapState's camera data to set Entity's on-screen position.
	*/

	TileEntity * te = TileEntity(h);
	Entity * e = Entity(h);

	e->x = iso_x(te->mapstate, te->x, te->y);
	e->y = iso_y(te->mapstate, te->x, te->y);
	
	//    Entity sub-cell offset
	// TODO: make sensitive to tile widths
	e->x += (te->offset_x - te->offset_y);
	e->y += (te->offset_x + te->offset_y)/2;
	e->y += te->offset_z;

	entityOnDraw(h);
}

void tileEntitySetTile(TileEntity * e, int x, int y, int layer){
	MapStateTile * old_tile = getTile(e->mapstate->map,x,y);

	if(old_tile && old_tile->contents[layer] == e)
		old_tile->contents[layer] = NULL;

	e->x = x; e->y = y;
	MapStateTile * tile = getTile(e->mapstate->map,x,y);
	tile->contents[layer] = e;
}
