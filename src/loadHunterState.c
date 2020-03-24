#include "loadHunterState.h"
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include "userdata.h"
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include "hunter.h"
#include <string.h>
#include "sprites.h"

LoadHunterState * makeLoadHunterState(LoadHunterState * state, Hunter * hunter){
	if (state == NULL)
		state = (LoadHunterState *) calloc(sizeof(LoadHunterState), 1);

	state->selector = 0;
	state->scroll = 0;

	if (hunter == NULL)
		hunter = (Hunter*) calloc(sizeof(Hunter), 1);
	state->hunter = hunter;

	// Default values
	state->rect.x = 32;
	state->rect.y = 72;
	state->rect.w = 640 - 64;
	state->rect.h = 480 - 64 - 156 - 16;
	state->max_rows = (state->rect.h - 32) / textures.font.h;

	EventHandler(state)->type = "LoadHunterState";
	EventHandler(state)->onPop = loadHunterStateOnPop;
	EventHandler(state)->onPush = loadHunterStateOnPush;
	EventHandler(state)->onDraw = loadHunterStateOnDraw;
	EventHandler(state)->onKeyUp = loadHunterStateOnKeyUp;

	return state;
}

void loadHunterStateOnPop(EventHandler * h){
	struct HunterFileLinkedList * node = LoadHunterState(h)->hunter_list;
	while (node != NULL){
		struct HunterFileLinkedList * next = node->next;
		free(node);
		node = next;
	}
}

void loadHunterStateOnPush(EventHandler * h){
	char buffer[256];

	// Iterate over hunters directory
	DIR * dir;
	struct dirent * ent;
	dir = opendir(dataPath((char*) &buffer, "hunters"));
	// TODO: opendir errors

	struct HunterFileLinkedList * hunters = NULL;

	while ((ent = readdir(dir)) != NULL) {
		char * fname = ent->d_name;

		// Skip hidden files, as well as current and parent directories
		if (fname[0] == '.')
			continue;

		// Get the absolute path of the hunter file
		sprintf(
				(char*) &buffer, "%s/%s",
				dataPath((char*) &buffer, "hunters"),
				ent->d_name
			);
		
		struct HunterFileLinkedList * node = (struct HunterFileLinkedList*) calloc(sizeof(struct HunterFileLinkedList), 1);
		strcpy((char*) &node->fpath, (char*) &buffer);

		// Stat hunter
		if(stat((char*) &buffer, &node->attr) != 0){
			// TODO: error
		}

		// Sort hunters by modified date, newest first

		// If root node is empty, set it to the current node and continue to next node
		if (hunters == NULL) {
			hunters = node;
			continue;
		}

		// If the new node's modify time is greatest, make it the new root and continue
		// TODO: replace ifdef's with something more clean
		time_t node_time =
#ifdef _WIN32
			node->attr.st_mtime;
#else
			node->attr.st_mtim.tv_sec;
#endif

		if (
#ifdef _WIN32
				node_time >= hunters->attr.st_mtime
#else
				node_time >= hunters->attr.st_mtim.tv_sec
#endif
		) {
			hunters->prev = node;
			node->next = hunters;
			hunters = node;
			continue;
		}

		// Search for sorted insert point for node
		struct HunterFileLinkedList * iter = hunters;

		while (iter->next != NULL) {
#ifdef _WIN32
			time_t iter_time = iter->attr.st_mtime;
			time_t next_time = iter->next->attr.st_mtime;
#else
			time_t iter_time = iter->attr.st_mtim.tv_sec;
			time_t next_time = iter->next->attr.st_mtim.tv_sec;
#endif

			if(iter_time >= node_time && node_time >= next_time)
				break;

			iter = iter->next;
		}

		// Insert node, maintain list links
		node->prev = iter;
		node->next = iter->next;
		if(iter->next)
			iter->next->prev = node;;
		iter->next = node;
	}
	closedir(dir);

	// Load Hunters

	struct HunterFileLinkedList * iter = hunters;
	for(; iter; iter = iter->next){
		// Load hunter file contents
		FILE * fd = fopen((char*) &iter->fpath, "rb");
		fread((char*) &buffer, sizeof(buffer), 1, fd);
		fclose(fd);

		decodeHunter(&iter->hunter, (char*) &buffer);
	}

	((LoadHunterState *) h)->hunter_list = hunters;
}

void loadHunterStateOnDraw(EventHandler * h){
	LoadHunterState * state = LoadHunterState(h);

	prevStateOnDraw(h);

	// Animate panel expansion, exit if not finished
	float expand_time = 175;
	float scale = (float) GameState(state)->duration / expand_time;
	if(scale < 1.0){
		drawWindowPanelScaled(state->color, &state->rect, scale);
		return;
	}
	drawWindowPanel(state->color, &state->rect);
	
	// Draw hunter names
	int margin = 16;
	int y = 0;

	struct HunterFileLinkedList * node = state->hunter_list;
	for (; node != NULL; y++, node = node->next) {
		// Skip drawing Hunter names from outside of scrolled view
		if (y < state->scroll)
			continue;

		if (y > state->scroll + state->max_rows)
			continue;

		drawString(
				(char*) &node->hunter.name,
				state->rect.x + margin + 16,
				(y - state->scroll) * textures.font.h + state->rect.y + margin
			);
	}

	// Draw a little square left of the name of the currently selected Hunter's name
	SDL_Rect selector = {
		state->rect.x + margin + textures.font.w / 4,
		(state->selector - state->scroll) * textures.font.h + state->rect.y + margin + textures.font.h / 2 - textures.font.w / 4,
		textures.font.w / 2,
		textures.font.w / 2
	};
	SDL_RenderFillRect(game.renderer, &selector);
}

void loadHunterStateOnKeyUp(EventHandler * h, SDL_Event * e){
	LoadHunterState * state = LoadHunterState(h);

	// Find currently-selected hunter
	struct HunterFileLinkedList * node = state->hunter_list;
	for(int i = state->selector; i > 0; i--)
		node = node->next;

	switch (e->key.keysym.scancode) {
		case SDL_SCANCODE_UP:
			if(node->prev)
				state->selector--;
			break;

		case SDL_SCANCODE_DOWN:
			if (node->next)
				state->selector++;
			break;

		case SDL_SCANCODE_SPACE:
		case SDL_SCANCODE_RETURN:
			memcpy(state->hunter, &node->hunter, sizeof(Hunter));
			gamePopState();
			break;

		default:
			break;
	}

	if (state->selector - state->scroll < 0) {
		state->scroll = state->selector;
	}
	else if (state->selector > state->scroll + state->max_rows) {
		state->scroll = state->selector - state->max_rows;
	}

}
