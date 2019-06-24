#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "stateengine.h"
#include "mapstate.h"
#include "animations.h"

extern Game game;
extern MapState mapstate;
extern MenubarState menubar;
extern StatpanelState statpanel;

Map * makeMap(int w, int h){
	Map * ret = (Map*) malloc(sizeof(Map));
	ret->w = w;
	ret->h = h;
	ret->tiles = (Tile*) calloc(sizeof(Tile), w*h);

	for(int n=0; n < w*h; n++){
		ret->tiles[n].x = n % w;
		ret->tiles[n].y = n / w;
		ret->tiles[n].tint_r = 255;
		ret->tiles[n].tint_g = 255;
		ret->tiles[n].tint_b = 255;
	}

	return ret;
}

MapState * makeMapState(MapState * mapstate, int map_w, int map_h){
	if(mapstate == NULL)
		mapstate = MapState(calloc(sizeof(MapState), 1));

	mapstate->map = makeMap(map_w, map_h);
	mapstate->menubar = MenubarState(calloc(sizeof(MenubarState), 1));

	mapstate->tile_w = 64;
	mapstate->tile_h = 32;
	mapstate->tile_src_w = 32;
	mapstate->tile_src_h = 16;
	mapstate->tile_img_h = 32;

	mapstate->card_w = 32;
	mapstate->card_h = 36;
	mapstate->card_src_w = 16;
	mapstate->card_src_h = 18;

	mapstate->statbox_tile_w = 16;
	mapstate->statbox_tile_h = 18;
	mapstate->statbox_tile_src_w = 8;
	mapstate->statbox_tile_src_h = 9;
	
	// TODO: calculate center
	mapstate->camera_x = 280;
	mapstate->camera_y = 125;

	// Load some textures
	mapstate->cards_texture = IMG_LoadTexture(
			game.renderer, "resources/cards.png");
	mapstate->tiles_texture = IMG_LoadTexture(
			game.renderer, "resources/hunter-tile.png");
	mapstate->daniel_texture = IMG_LoadTexture(
			game.renderer, "resources/daniel.png");
	mapstate->statbox_texture = IMG_LoadTexture(
			game.renderer, "resources/statbox.png");

	mapstate->menubar->background_texture = IMG_LoadTexture(
			game.renderer, "resources/menubar-gradient.png");
	mapstate->menubar->buttons_map = IMG_LoadTexture(
			game.renderer, "resources/menu-battle-icons.png");

	EventHandler(mapstate)->onTick = mapOnTick;
	EventHandler(mapstate)->onMouseDown = mapOnMouseDown;
	EventHandler(mapstate)->onDraw = mapOnDraw;
	EventHandler(mapstate)->onKeyUp = mapOnKeyUp;

	EventHandler(mapstate->menubar)->onDraw = menuOnDraw;

	pushAction(mapstate, "poll_turn_action");
	return mapstate;
}

void drawWindowPanel(MapState * state, enum WindowColor color, SDL_Rect * window_dest){
	int tile_w = 8;
	int tile_h = 9;
	
	// Draw panel fill
	SDL_SetRenderDrawBlendMode(game.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(game.renderer, 255-32, 255, 255, 32);
	SDL_RenderFillRect(game.renderer, window_dest);
	SDL_SetRenderDrawBlendMode(game.renderer, SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(game.renderer, 255, 255, 255, 255);
	
	// Panel inset border
	SDL_Rect border = *window_dest;
	border.x += 2; border.y += 2;
	border.w -= 4; border.h -= 4;
	SDL_RenderDrawRect(game.renderer, &border);

	//Draw bar middle
	SDL_Rect src = {tile_w, tile_h*2 + tile_h* color, tile_w, tile_h};
	SDL_Rect dest = {window_dest->x, window_dest->y, window_dest->w, tile_w*2};
	blit(state->statbox_texture, &src, &dest);
	
	// Draw bar left
	src.x = 0;
	dest.w = tile_w * 2;
	blit(state->statbox_texture, &src, &dest);

	// Draw bar right
	src.x = tile_w * 2;
	dest.x += window_dest->w - tile_w * 2;
	blit(state->statbox_texture, &src, &dest);
}

void mapSetSelection(Map * map, int value){
	for(int n=0; n < map->w*map->h; n++)
		map->tiles[n].selected = value;
}

void mapSelectAll(Map * map){  mapSetSelection(map, 1); }
void mapSelectNone(Map * map){ mapSetSelection(map, 0); }
void mapSelectRange(Map * map, int c_x, int c_y, int range){
	for_xy(x, y, map->w, map->h){
		Tile * tile = getTile(map, x,y);
		tile->selected = (abs(c_x - x) + abs(c_y - y)) <= range;
	}
}

void freeMap(Map * map){
	free(map->tiles);
	free(map);
}

Tile * getTile(Map * map, int x, int y){
	return &(map->tiles[y * map->w + x]);
}

Tile * getTileAtPx(MapState * state, float p_x, float p_y){
	// Account for the tile's diagonal edges
	p_x -= state->tile_w / 4;
	p_y -= state->tile_h / 4;

	// Tile cell position
	int tx = p_x/state->tile_w + p_y/state->tile_h;
	int ty = p_y/state->tile_h - p_x/state->tile_w;

	
	// Return NULL if pixel position is outside of map boundaries
	if((tx < 0) | (tx > state->map->w)) return NULL;
	if((ty < 0) | (ty > state->map->h)) return NULL;

	return getTile(state->map, tx, ty);
}

void entityOnDraw(EventHandler * h){
	Entity * entity = Entity(h);
	AnimationFrame * frame = entity->animation_frame;

	if(frame == NULL)
		frame = entity->animation_frame = entity->animation;

	// Handle animation timing
	if(frame){
		uint32_t time = SDL_GetTicks();
		
		// If the entity has no animation timing data, create
		// it and don't update to the next frame.
		if(entity->last_frame == 0){
			entity->last_frame = time;
		}

		// If it is time, update the frame, but only if there
		// is a new frame to update to.
		else if(time - entity->last_frame >= frame->duration){
			if(entity->animation_frame->next)
				entity->animation_frame = entity->animation_frame->next;
			else if (entity->animation_loop){
				frame = entity->animation_frame = entity->animation;
			}
			entity->last_frame = time;
		}
	}
	
	// Handle flips
	uint8_t flip_v=0, flip_h=0;
	flip_h = entity->flip_h;
	flip_v = entity->flip_v;

	if(frame){
		flip_h ^= frame->flip_h;
		flip_v ^= frame->flip_v;
	}
	
	SDL_RendererFlip flip = SDL_FLIP_NONE;
	if(flip_h) flip |= SDL_FLIP_HORIZONTAL;
	if(flip_v) flip |= SDL_FLIP_VERTICAL;

	// Handle source clipping
	SDL_Rect * src = NULL;
	if(frame)
		src = &frame->clip;

	// Rendering destination
	int src_w = 1;
	int src_h = 1;

	if(src) src_w = src->w;
	if(src) src_h = src->h;

	SDL_Rect dest = {
			0, 0, src_w*entity->scale_w, src_h*entity->scale_h
		};

	//    Map cell coordinates
	dest.x = iso_x(entity->mapstate, entity->x, entity->y);
	dest.y = iso_y(entity->mapstate, entity->x, entity->y);
	
	//    Entity sub-cell offset
	// TODO: make sensitive to tile widths
	dest.x += (entity->offset_x - entity->offset_y);
	dest.y += (entity->offset_x + entity->offset_y)/2;

	//    Frame center
	if(frame){
		dest.x -= frame->center_x * entity->scale_w;
		dest.y -= frame->center_y * entity->scale_h;
	}

	// Render
	SDL_RenderCopyEx(
			game.renderer, entity->texture,
			src, &dest,
			0, NULL,  // Rotation not currently supported
			flip
		);
}

void entitySetAnimation(Entity * entity, AnimationFrame * animation){
	if(entity->animation != animation){
		entity->animation = animation;
		entity->animation_frame = NULL;
	}
}

void entitySetTile(Entity * e, int x, int y, int layer){
	Tile * old_tile = getTile(e->mapstate->map,x,y);

	if(old_tile && old_tile->contents[layer] == e)
		old_tile->contents[layer] = NULL;

	e->x = x; e->y = y;
	Tile * tile = getTile(e->mapstate->map,x,y);
	tile->contents[layer] = e;
}

void hunterSetTile(HunterEntity * e, int x, int y){
	entitySetTile(Entity(e), x, y, TILE_LAYER_HUNTER);
}

HunterEntity * initHunter(HunterEntity * hunter, MapState * state, SDL_Texture * texture){
	if(hunter == NULL)
		hunter = HunterEntity(malloc(sizeof(HunterEntity)));
	
	Entity * e = Entity(hunter);
	e->mapstate = state;
	e->texture = texture;
	e->animation = (AnimationFrame*) &ANIM_HUNTER_STAND_S;
	e->animation_loop = 1;
	e->scale_w = 2;
	e->scale_h = 2;
	EventHandler(hunter)->onDraw = entityOnDraw;
	return hunter;
}

void mapOnTick(EventHandler * h){
	MapState * state = MapState(h);
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	
	if(pollAction(state, "poll_tile_select")){
		// Pan camera with arrow keys
		if(keys[SDL_SCANCODE_UP])    state->camera_y += 10;
		if(keys[SDL_SCANCODE_DOWN])  state->camera_y -= 10;
		if(keys[SDL_SCANCODE_LEFT])  state->camera_x += 10;
		if(keys[SDL_SCANCODE_RIGHT]) state->camera_x -= 10;

		Statset * stats = hunterStats(state->daniel->hunter);
		mapSelectRange(
				state->map,
				Entity(state->daniel)->x,
				Entity(state->daniel)->y,
				stats->mov + 1
			);
	}

	else if(pollAction(state, "move_entity")){
		int target_x = state->action->target_x;
		int target_y = state->action->target_y;
		int speed = 6;
		Entity * entity = Entity(state->daniel);

		if(entity->x > target_x){
			entity->direction = WEST;
			entity->flip_h = 1;
			entitySetAnimation(
					entity,
					(AnimationFrame *) &ANIM_HUNTER_RUN_N
				);

			entity->offset_x -= speed;
			if(entity->offset_x <= -state->tile_w/2){
				entity->offset_x = 0;
				entity->x--;
			}
		}
		else if(entity->x < target_x){
			entity->direction = EAST;
			entity->flip_h = 1;
			entitySetAnimation(
					Entity(entity),
					(AnimationFrame *) &ANIM_HUNTER_RUN_S
				);

			entity->offset_x += speed;
			if(entity->offset_x >= state->tile_w/2){
				entity->offset_x = 0;
				entity->x++;
			}
		}
		else if(entity->y > target_y){
			entity->direction = NORTH;
			entity->flip_h = 0;
			entitySetAnimation(
					entity,
					(AnimationFrame *) &ANIM_HUNTER_RUN_N
				);

			entity->offset_y -= speed;
			if(entity->offset_y <= -state->tile_h){
				entity->offset_y = 0;
				entity->y--;
			}
		}
		else if(entity->y < target_y){
			entity->direction = SOUTH;
			entity->flip_h = 0;
			entitySetAnimation(
					entity,
					(AnimationFrame *) &ANIM_HUNTER_RUN_S
				);

			entity->offset_y += speed;
			if(entity->offset_y >= state->tile_h){
				entity->offset_y = 0;
				entity->y++;
			}
		}

		hunterSetTile(state->daniel, entity->x, entity->y);

		if(
			(entity->x == target_x) &&
			(entity->y == target_y)
		){
			popAction(state);
			state->daniel->hunter->turn_stats.mov = 0;
			switch(entity->direction){
				case NORTH:
				case WEST:
					entitySetAnimation(
							entity,
							(AnimationFrame *) &ANIM_HUNTER_STAND_N
						);
					break;
				case SOUTH:
				case EAST:
					entitySetAnimation(
							entity,
							(AnimationFrame *) &ANIM_HUNTER_STAND_S
						);
					break;
			}
		}
	}

	// Forward event to menubar
	state->menubar->active = pollAction(state, "poll_turn_action");

	EventHandler * menu_handler = EventHandler(state->menubar);
	if(menu_handler && menu_handler->onTick)
		menu_handler->onTick(menu_handler);
}

void mapOnKeyUp(EventHandler * h, SDL_Event * e){
	MapState * state = MapState(h);

	// Menubar arrow keys
	if(pollAction(state, "poll_turn_action")){
		switch(e->key.keysym.scancode){
			case SDL_SCANCODE_LEFT:
				state->menubar->selector--;
				if(state->menubar->selector == -1)
					state->menubar->selector = 4;
				break;

			case SDL_SCANCODE_RIGHT:
				state->menubar->selector++;
				if(state->menubar->selector >= 5)
					state->menubar->selector = 0;
				break;

			case SDL_SCANCODE_SPACE:
				state->card_selected = 0;
				pushAction(state, "poll_move_card_select");
				break;

			default:
				break;
		}
	}
	
	// Select card
	else if(pollAction(state, "poll_move_card_select")){

		// Calculate hand size
		int hand_size = hunterHandSize(state->daniel->hunter);
		Card * card = NULL;
		
		switch(e->key.keysym.scancode){
			case SDL_SCANCODE_LEFT:
				state->card_selected--;
				if(state->card_selected == -1)
					state->card_selected = hand_size - 1;
				break;

			case SDL_SCANCODE_RIGHT:
				state->card_selected++;
				if(state->card_selected >= hand_size)
					state->card_selected = 0;
				break;

			case SDL_SCANCODE_SPACE:
				card = hunterPopCard(state->daniel->hunter, state->card_selected);
				state->daniel->hunter->turn_stats.mov = card->num;
				popAction(state);
				pushAction(state, "poll_tile_select");
				break;

			case SDL_SCANCODE_ESCAPE:
				popAction(state);
				break;

			default:
				break;
		}
	}

	
	else if(pollAction(state, "poll_tile_select")){
		if(e->key.keysym.scancode == SDL_SCANCODE_ESCAPE)
			popAction(state);
	}
}

void mapOnMouseDown(EventHandler * h, SDL_Event * e){
	MapState * state = MapState(h);
	SDL_MouseButtonEvent me = e->button;

	Tile * t = getTileAtPx(
			state,
			me.x - state->camera_x,
			me.y - state->camera_y
		);

	if(t){
		if(pollAction(state, "poll_tile_select")){
			if((t->selected) && (t->val)){
				popAction(state);
				mapSelectNone(state->map);
				ActionQueue * action = pushAction(state, "move_entity");
				action->target_x = t->x;
				action->target_y = t->y;
			}
		}
	}
}

int iso_x(MapState * state, int x, int y){
	return state->camera_x + (x-y) * state->tile_w/2 + state->tile_w/2;
};

int iso_y(MapState * state, int x, int y){
	return state->camera_y + (x+y) * state->tile_h/2 + state->tile_h/2;
};

void mapOnDraw(EventHandler * h){
	MapState * state = MapState(h);
	Map * map = state->map;
	
	SDL_Rect srcrect  = {0, 0, state->tile_src_w, state->tile_img_h};

	SDL_Rect destrect = {
			0, 0,
			state->tile_w,
			state->tile_h * state->tile_img_h / state->tile_src_h
		};
	

	// Draw tiles
	for_xy(x, y, map->w, map->h){
		Tile * tile = getTile(map, x, y);
		
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
		Tile * tile = getTile(map, x, y);

		for(int e=0; e < TILE_ENTITY_LAYERS; e++){
			Entity * entity = tile->contents[e];

			if(entity == NULL)
				continue;

			if(EventHandler(entity))
				EventHandler(entity)->onDraw(EventHandler(entity));
		}
	}

	// TODO: draw screen tint

	if(pollAction(state, "poll_move_card_select")){

		// Draw card select
		SDL_Rect window_panel = {32, 76, 16 + state->card_w * 7, 64};
		drawWindowPanel(state, WINDOW_BLUE, &window_panel);

		for(int c=0; c < HAND_LIMIT; c++){
			Card * card = state->daniel->hunter->hand[c];
			
			if(card == NULL)
				break;

			SDL_Rect src = {0, 0, state->card_src_w, state->card_src_h};
			SDL_Rect dest = {
					window_panel.x + 8 + c*state->card_w,
					window_panel.y + 16,
					state->card_w, state->card_h
				};

			src.x = card->num * src.w;

			if(state->card_selected == c)
				dest.y += 8;

			switch(card->type){
				case NULL_CARD:
				case MOVE_CARD:         
				case MOVE_EXIT_CARD:    
					src.y = 0;
					break;

				case DEFENSE_CARD:      
				case DEFENSE_ALL_CARD:  
				case DEFENSE_DOUBLE_CARD:
					src.y = src.h * 1;
					break;

				case ATTACK_CARD:       
				case ATTACK_DOUBLE_CARD:
				case ATTACK_COPY_CARD:  
					src.y = src.h * 2;
					break;

				case EMPTY_TRAP_CARD:   
				case STUN_TRAP_CARD:    
				case DAMADE_TRAP_CARD:  
				case LEG_DAMAGE_CARD:   
					src.y = src.h * 2;
					break;
			}

			blit(state->cards_texture,  &src, &dest);
		}
	}
	
	// Forward event to menubar
	EventHandler * menu_handler = EventHandler(state->menubar);
	if(menu_handler && menu_handler->onDraw)
		menu_handler->onDraw(menu_handler);

	// Draw stat windows, character stats
	int panel_gutter = 4;
	int panel_w = (640 - 16*2 - 4*3) / 4;
	for(int h=0; h < HUNTERS_COUNT; h++){
		Hunter * hunter = state->hunters[h].hunter;
		Statset * stats = hunterStats(hunter);
		
		// Draw panel
		SDL_Rect panel_rect = {
				16 + (panel_w + panel_gutter)*h, 480 - 160 - panel_gutter,
				panel_w, 160
			};
		drawWindowPanel(state, h, &panel_rect);

		int element_gutter = 8;
		int element_margin = 18;
		
		// Draw stat names
		//    Mv. stat
		SDL_Rect statname_src = {
				0, state->statbox_tile_src_h,
				state->statbox_tile_src_w * 2,     // takes up two tiles
				state->statbox_tile_src_h
			};

		SDL_Rect statname_dest = {
				panel_rect.x + element_margin,
				panel_rect.y + (element_gutter + state->statbox_tile_h) * 2,
				state->statbox_tile_w * 2,
				state->statbox_tile_h
			};

		blit(state->statbox_texture, &statname_src, &statname_dest);

		//    Mv. +
		SDL_Rect statval_src  = statname_src;
		statval_src.x = state->statbox_tile_src_w * 8;
		statval_src.w = state->statbox_tile_src_w;

		SDL_Rect statval_dest = statname_dest;
		statval_dest.y += state->statbox_tile_h + 2;
		statval_dest.w = state->statbox_tile_w;
		blit(state->statbox_texture, &statval_src, &statval_dest);

		//    Mv. #
		statval_dest.x += state->statbox_tile_w;
		statval_src.y = 0;
		statval_src.x = stats->mov * state->statbox_tile_src_w;
		blit(state->statbox_texture, &statval_src, &statval_dest);

		//    At. stat label
		statname_src.x += state->statbox_tile_src_w * 2;
		statname_dest.x += state->statbox_tile_w*2 + element_gutter;
		blit(state->statbox_texture, &statname_src, &statname_dest);

		//    At. stat 10's digit
		statval_dest.x = statname_dest.x;
		if(stats->atk / 10){
			statval_src.x = (stats->atk/10) * state->statbox_tile_src_w;
			blit(state->statbox_texture, &statval_src, &statval_dest);
		}
		//    At. stat 1's digit
		statval_dest.x += state->statbox_tile_w;
		statval_src.x = (stats->atk % 10) * state->statbox_tile_src_w;
		blit(state->statbox_texture, &statval_src, &statval_dest);

		//    Df. stat label
		statname_src.x += state->statbox_tile_src_w * 2;
		statname_dest.x += state->statbox_tile_w*2 + element_gutter;
		blit(state->statbox_texture, &statname_src, &statname_dest);

		//    Df. stat 10's digit
		statval_dest.x = statname_dest.x;
		if(stats->def / 10){
			statval_src.x = (stats->atk/10) * state->statbox_tile_src_w;
			blit(state->statbox_texture, &statval_src, &statval_dest);
		}
		//    Df. stat 1's digit
		statval_dest.x += state->statbox_tile_w;
		statval_src.x = (stats->def % 10) * state->statbox_tile_src_w;
		blit(state->statbox_texture, &statval_src, &statval_dest);
	}
}

void menuOnDraw(EventHandler * h){
	MenubarState * menu = MenubarState(h);

	// Render menubar
	SDL_Rect src = {0, 0, 1, 64};
	SDL_Rect dest = {0, 0, 640, 64};
	
	blit(menu->background_texture, &src, &dest);
	
	// Render left icons
	src.w  = 16; src.h  = 16;
	dest.y = 64 - 32 - 8;
	dest.w = 32; dest.h = 32;

	for(int i=0; i<5; i++){
		src.x = i * 16;
		dest.x = i * 38 + 32;
		blit(menu->buttons_map, &src, &dest);
	}

	// Draw deck icon
	src.x = 5 * 16;
	dest.x = 640 - 32 * 4;
	blit(menu->buttons_map, &src, &dest);

	// Draw scroling text window
	src.x = 0; src.y = 16;
	src.w = 144;
	src.h = 16;
	dest.x = 5 * 38 + 32;
	dest.w = 144*2;
	blit(menu->buttons_map, &src, &dest);

	// Draw selector feather
	if(menu->active){
		src.x = 0; src.y = 32;
		src.w = 32; src.h = 32;
		
		if(menu->selector != -1){
			dest.x = menu->selector * 38 + 32;
			dest.w = 64; dest.h = 64;
			blit(menu->buttons_map, &src, &dest);
		}
	}
}

ActionQueue * makeAction(char * type){
	ActionQueue * ret = (ActionQueue*) malloc(sizeof(ActionQueue));
	ret->type = type;
	return ret;
}

ActionQueue * pushAction(MapState * state, char * type){
	ActionQueue * new_action = makeAction(type);
	new_action->next = state->action;
	new_action->start = SDL_GetTicks();
	state->action = new_action;
	return new_action;
}

int pollAction(MapState * state, char * type){
	if(state->action == NULL)
		return 0;

	return strcmp(state->action->type, type) == 0;
}

void popAction(MapState * state){
	ActionQueue * pop = state->action;
	state->action = state->action->next;
	free(pop);
}
