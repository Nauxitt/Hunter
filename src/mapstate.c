#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "stateengine.h"
#include "mapstate.h"
#include "combatstate.h"
#include "animations.h"
#include "sprites.h"
#include "draw.h"
#include "entity.h"
#include "menubar.h"
#include "utils.h"

#include "handstate.h"
#include "dicestate.h"

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
			case POLL_MOVE_CARD_ACTION:
			case POLL_MOVE_ACTION:
			case POLL_ATTACK_ACTION:
			case POLL_COMBAT_CARD_ACTION:
			case POLL_COMBAT_ACTION:
			case POLL_TURN_ACTION:
			case POLL_DEFEND_ACTION:
				breaker = 0;

			case BEGIN_MATCH_ACTION:
			case TURN_START_ACTION:
			case TURN_END_ACTION:
			case DRAW_CARD_ACTION:
			case HEAL_ACTION:
			case MOVE_ACTION:
			case ATTACK_ACTION:
			case REST_ACTION:
			case DEFEND_ACTION:
			case ESCAPE_ACTION:
			case SURRENDER_ACTION:
			case USE_CARD_ACTION:
			case CATCH_ROLL_BONUS_ACTION:
			case ESCAPE_ROLL_BONUS_ACTION:
			case ATTACK_ROLL_BONUS_ACTION:
			case DEFENSE_ROLL_BONUS_ACTION:
			case TELEPORT_RANDOM_ACTION:
			case COMBAT_ACTION:
			case EXIT_COMBAT_ACTION:
			case EXECUTE_COMBAT_ACTION:
			case DEATH_CHECK_ACTION:
			case REMOVE_RELIC_ACTION:
			case DAMAGE_ACTION:
			case ROLL_DICE_ACTION:
			case ATTACK_DAMAGE_ACTION:
				matchCycle(match);
				break;

			case TELEPORT_ACTION:
				hunterSetTile(action_hunter_entity, action->x, action->y);
				matchCycle(match);
				matchCycle(match);
				break;

			case MOVE_ROLL_BONUS_ACTION:
				gamePushState(GameState(initDiceState(
						NULL, match, match->dice[0],
						game.w/2,
						game.h/2 - textures.dice.h/2,
						MOVE_DICE_COLOR
					)));
				return;

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

			case ENTER_COMBAT_ACTION:
				mapEnterCombat(state);
				break;

			case MOVE_STEP_ACTION:
				mapMoveHunter(
						state, action_hunter_entity,
						action->x, action->y, 6
					);
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
		}
	}

	MatchAction * action = match->action;

	if(action->type == POLL_MOVE_ACTION){
		if(!pollAction("poll_move_action")){
			pushAction("poll_move_action");
			mapStateFlash(state);
		}
		
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
				stats->mov
			);
	}
	else if(
				pollAction("poll_combat_target") ||
				(action->type == POLL_COMBAT_ACTION)
			){
		// Pan camera with arrow keys
		if(keys[SDL_SCANCODE_UP])    state->camera_y += 10;
		if(keys[SDL_SCANCODE_DOWN])  state->camera_y -= 10;
		if(keys[SDL_SCANCODE_LEFT])  state->camera_x += 10;
		if(keys[SDL_SCANCODE_RIGHT]) state->camera_x -= 10;
		
		// Select adjacent tiles which contain hunters

		int x = action->actor->x, y = action->actor->y;
		MapStateTile * tile;

		tile = getTile(state->map, x, y-1);
		if(tile)
			tile->selected = (int) getHunterAt(match, x, y-1);

		tile = getTile(state->map, x, y+1);
		if(tile)
			tile->selected = (int) getHunterAt(match, x, y+1);

		tile = getTile(state->map, x-1, y);
		if(tile)
			tile->selected = (int) getHunterAt(match, x-1, y);

		tile = getTile(state->map, x+1, y);
		if(tile)
			tile->selected = (int) getHunterAt(match, x+1, y);
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

	// Toggle statboxron TAB
	if(e->key.keysym.scancode == SDL_SCANCODE_TAB){
		state->statbox_view++;
		if(state->statbox_view == STATBOX_VIEW_NONE + 1)
			state->statbox_view = 0;
	}

	// Escape prints a debug message of the current match action
	else if(e->key.keysym.scancode == SDL_SCANCODE_BACKSPACE){
		printf("Backspace: ");
		printMatchAction(match->action);
	}

	// Menubar arrow keys
	if(action->type == POLL_TURN_ACTION){
		if(pollAction("poll_turn_action")){
			switch(e->key.keysym.scancode){
				case SDL_SCANCODE_LEFT:
				case SDL_SCANCODE_RIGHT:
					onKeyUp(EventHandler(state->menubar), e);
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
						pushAction("poll_combat_target");
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

	else if(pollAction("poll_combat_target")){
		if(e->key.keysym.scancode == SDL_SCANCODE_ESCAPE){
			nextAction();
			mapSelectNone(state->map);
		}
	}

	else if(action->type == POLL_COMBAT_ACTION){
		if(e->key.keysym.scancode == SDL_SCANCODE_ESCAPE){
			postCombatAction(match, action->actor, NULL);
			mapSelectNone(state->map);
		}
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

	if(t && t->selected && t->val){
		if(match->action->type == POLL_MOVE_ACTION){
			if(pollAction("poll_move_action"))
				nextAction();

			postMoveAction(
					match, NULL, t->x, t->y
				);
			mapSelectNone(state->map);
		}

		else if(
					(match->action->type == POLL_COMBAT_ACTION) ||
					pollAction("poll_combat_target")
				){
			Hunter * target = getHunterAt(match, t->x, t->y);
			postCombatAction(match, match->action->actor, target);
			mapSelectNone(state->map);
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

	// Draw screen tint
	if(state->tint.a > 0){
		SDL_Rect dest = {0, 0, game.w, game.h};
		SDL_SetRenderDrawBlendMode(game.renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(
				game.renderer,
				state->tint.r,
				state->tint.g,
				state->tint.b,
				state->tint.a
			);
		SDL_RenderFillRect(game.renderer, &dest);
		SDL_SetRenderDrawBlendMode(game.renderer, SDL_BLENDMODE_NONE);
	}
	
	// Iterate through tiles and draw entities
	for_xy(x, y, map->w, map->h){
		MapStateTile * tile = getTile(map, x, y);

		for(int e=0; e < TILE_ENTITY_LAYERS; e++){
			TileEntity * entity = tile->contents[e];

			if(entity == NULL)
				continue;

			onDraw(EventHandler(entity));
		}
	}
	
	// Forward event to menubar
	if(state->menubar)
		onDraw(EventHandler(state->menubar));

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
	
	if(te->mapstate == NULL){
		e->x = te->x - te->y;
		e->y = (te->x + te->y)/2;
	}
	else {
		e->x = iso_x(te->mapstate, te->x, te->y);
		e->y = iso_y(te->mapstate, te->x, te->y);
	}
	
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

void mapEnterCombat(MapState * state){
	MatchContext * match = state->match;
	matchCycle(match);

	CombatState * combat = makeCombatState(NULL, match);
	combat->menubar = state->menubar;
	combat->menubar->selector = -1;
	
	for(int n = 0; n < 4; n++)
		if(state->hunters[n].hunter == match->attacker){
			HunterEntity * hunter = &state->hunters[n];
			HunterEntity * dest = &combat->attacker_entity;
			memcpy(dest, hunter, sizeof(HunterEntity));
			EventHandler(dest)->onDraw = entityOnDraw;
			break;
		}

	for(int n = 0; n < 4; n++)
		if(state->hunters[n].hunter == match->defender){
			HunterEntity * hunter = &state->hunters[n];
			HunterEntity * dest = &combat->defender_entity;
			memcpy(dest, hunter, sizeof(HunterEntity));
			EventHandler(dest)->onDraw = entityOnDraw;
			break;
		}

	gamePushState(GameState(combat));
}

void mapStateFlash(MapState * mapstate){
	/*
	   Pushes a state onto the stack which causes the MapState under it to draw a single white flash
	*/

	GameState * flashState = makeGameState();

	mapstate->tint.r = 255;
	mapstate->tint.g = 255;
	mapstate->tint.b = 255;

	EventHandler(flashState)->type = "MapState.flash";
	EventHandler(flashState)->onDraw = mapStateOnDrawFlash;
	gamePushState(flashState);
}

void mapStateOnDrawFlash(EventHandler * h){
	GameState * state = GameState(h);
	MapState * mapstate = MapState(state->prevState);

	int flash_duration = 75;
	int fade_duration = 250;
	int total_duration = flash_duration + fade_duration;

	if(state->duration <= flash_duration){
		mapstate->tint.a = state->duration * 255 / flash_duration;
	}
	else if(state->duration <= total_duration){
		int t = state->duration - flash_duration;
		mapstate->tint.a = (fade_duration - t) * 255 / fade_duration;
	}
	else {
		mapstate->tint.a = 0;
		gamePopState();
		free(h);
	}

	prevStateOnDraw(h);
}

HunterEntityDamageState * makeHunterEntityDamageState(HunterEntityDamageState * state, HunterEntity * hunter, int damage){
	if(state == NULL)
		state = (HunterEntityDamageState *) calloc(sizeof(HunterEntityDamageState), 1);

	state->hunter = hunter;
	state->damage = damage;
	state->old_animation = Entity(state->hunter)->animation;
	state->old_animation_loop = Entity(state->hunter)->animation_loop;
	state->bounce_duration = 250;
	state->bounce_height = 32;
	state->show_duration = 350;
	state->fade_duration = 150;

	EventHandler(state)->onEnter = hunterEntityDamageStateOnEnter;
	EventHandler(state)->onExit = hunterEntityDamageStateOnExit;
	EventHandler(state)->onDraw = hunterEntityDamageStateOnDraw;

	return state;
}

void hunterEntityDamageStateOnEnter(EventHandler * h){
	HunterEntityDamageState * state = (HunterEntityDamageState *) h;

	AnimationFrame * animation = (AnimationFrame*) &ANIM_HUNTER_DAMAGE_S;
	
	entitySetAnimation(Entity(state->hunter), animation);
	Entity(state->hunter)->animation_loop = 0;
	state->hit_end_time = animationGetDuration(animation);
}

void hunterEntityDamageStateOnExit(EventHandler * h){
	HunterEntityDamageState * state = (HunterEntityDamageState *) h;
	entitySetAnimation(Entity(state->hunter), state->old_animation);

	Entity(state->hunter)->animation_loop = state->old_animation_loop;
}
	
void hunterEntityDamageStateOnDraw(EventHandler * h){
	prevStateOnDraw(h);

	HunterEntityDamageState * state = (HunterEntityDamageState *) h;
	HunterEntity * hunter = state->hunter;

	// If the hit animation is done, draw damage
	if(GameState(h)->duration > state->hit_end_time){
		int number_time = GameState(h)->duration - state->hit_end_time;
		int number_duration = state->bounce_duration + state->show_duration;

		// Calculate number of digits to draw
		int digits = 0;
		for(int n=state->damage; n > 0; n/=10)
			digits++;

		if(digits == 0)
			digits = 1;

		// Horizontally center
		int x = Entity(hunter)->x + (digits-2)*textures.statbox.w/2;

		// Bounce digit
		int y = Entity(hunter)->y - 16;
		
		int bounce_time = GameState(h)->duration - state->hit_end_time;
		if(bounce_time < state->bounce_duration){
			float q = (float) bounce_time * 2 / (float) state->bounce_duration;
			y += state->bounce_height * (q*q - 2*q);
		}

		// Fade out digits after bouncing
		else {
			int fade_start = number_duration - state->fade_duration;
			if(number_time > fade_start){
				SDL_SetTextureAlphaMod(
						textures.statbox.texture,
						255 - 255 * (number_time - fade_start) / state->fade_duration
						);
			}
		}
		
		// Draw digits, right-to-left
		int n = state->damage;

		for(int p=digits; p > 0; p--){
			if(state->damage == 0)
				drawBigNumber(x, y, n % 10);
			else
				drawBigRedNumber(x, y, n % 10);
			n /= 10;
			x -= textures.statbox.w;
		}
		
		SDL_SetTextureAlphaMod(textures.statbox.texture, 255);
		
		// Is it time to exit this state?
		if(number_time > number_duration)
			free(gamePopState());
	}
}
