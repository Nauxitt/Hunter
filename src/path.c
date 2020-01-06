#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "path.h"
#include "hunter.h"
#include "utils.h"

void mapResetPathData(MatchContext * context){
	for (int x=0; x < context->map_w; x++) {
		for (int y=0; y < context->map_h; y++) {
			PathNode * node = &context->map[context->map_w*y + x].path;
			node->x = x;
			node->y = y;
			node->scanned = 0;
			node->distance = 0;
			node->to = NULL;
			node->from = NULL;
			node->next_path = NULL;
			node->prev_path = NULL;
		}
	}
}

PathNode * insertPath(PathNode * insert_point, PathNode * inserted) {
	if (insert_point) {
		inserted->next_path = insert_point->next_path;
		inserted->prev_path = insert_point->prev_path;
		insert_point->next_path->prev_path = inserted;
		insert_point->next_path = inserted;
	}
	else {
		// If this is the first element, establish it a circular list
		inserted->next_path = inserted;
		inserted->prev_path = inserted;
	}

	return inserted;
}

int inPath(PathNode * path, int x, int y ) {
	// If we've recursed past the start of the path
	if (path == NULL)
		return 0;

	// Check point position
	if ((path->x == x) && (path->y == y))
		return 1;

	// Recurse down path
	return inPath(path->to, x, y);
}

int inPathFrom(PathNode * endpoint, int x, int y ) {
	// If we've recursed past the start of the path
	if (endpoint == NULL)
		return 0;

	// Check point position
	if ((endpoint->x == x) && (endpoint->y == y))
		return 1;

	// Recurse down path
	return inPathFrom(endpoint->from, x, y);
}

PathNode * removePath(PathNode * path) {
	PathNode * insert_point = path->next_path;

	path->next_path->prev_path = path->prev_path;
	path->prev_path->next_path = path->next_path;
	
	path->next_path = NULL;
	path->prev_path = NULL;

	return insert_point;
}

void freePath(PathNode * path){
	while (path) {
		PathNode * next = path->to;
		free(path);
		path = next;
	}
}

PathNode * findPathWithin(MatchContext * context, int s_x, int s_y, int e_x, int e_y, int distance){
	PathNode * addNode(PathNode * from, int x, int y) {
		PathNode * node = &context->map[context->map_w * y + x].path;
		node->x = x;
		node->y = y;

		node->distance = 0;
		if (from) {
			node->from = from;
			node->distance = from->distance + 1;
		}
		return node;
	}
	
	// Clear leftover path data of previous searches from tile map
	mapResetPathData(context);

	// Start-point of the path is the initial node
	PathNode * path_head = addNode(NULL, s_x, s_y);
	insertPath(NULL, path_head);
	PathNode * final_path = NULL;

	while (path_head) {
		int x = path_head->x, y = path_head->y;
		path_head->scanned = 1;

		// If we've found the destination, nice, exit!
		if ((x == e_x) && (y == e_y)) {
			final_path = path_head;
			break;
		}

		// Expand paths clockwise, starting up, by maping a function in each
		// direction, unless we've hit the search distance limit.

		inline void expandPath(int x, int y) {
			PathNode * node = &context->map[context->map_w*y + x].path;

			if (node->scanned)                   return;
			if (!pointWalkable(context, x, y))   return;

			// Path doesn't break any requirements, add it
			insertPath(path_head->prev_path, addNode(path_head, x, y));
		}
		
		if (path_head->distance < distance) {
			ADJACENT_MAP(expandPath, x, y);
		}

		
		// Return NULL if we've scanned the entire map
		if (path_head->next_path == path_head)
			return NULL;

		// Remove the old path and go to the next
		path_head = removePath(path_head);
	}

	// Write to-path using from-path, then return the start of the whole path
	for (; final_path && final_path->from; final_path = final_path->from){
		final_path->from->to = final_path;
	}
	return final_path;
}

PathNode * findPath(MatchContext * context, int s_x, int s_y, int e_x, int e_y){
	return findPathWithin(context, s_x, e_y, e_x, e_y, INT_MAX);
}

int pathfindingMain(){
	Hunter hunters[] = {
		{	.name = "Daniel", .level = 1, .type = "hunter",
			.base_stats={.atk = 7, .mov = 4, .def = 3, .max_hp=1}
		},
		{	.name = "Dave", .level = 1, .type = "hunter",
			.base_stats = {.mov = 1, .atk = 11, .def = 1, .max_hp=1}
		},
		{	.name = "Stan", .level = 1, .type = "hunter",
			.base_stats = {.atk = 2, .mov = 3, .def = 8, .max_hp=1}
		},
		{	.name = "Tim", .level = 1, .type = "hunter",
			.base_stats = {.atk = 11, .mov = 1, .def = 1, .max_hp=1}
		}
	};

	Relic floppy = {.item_id=0, .name="floppy"};
	Relic metal = {.item_id=3, .name="metal"};

	Crate * crates = (Crate*) calloc(sizeof(Crate), 2);
	crates[0].exists = 1;
	crates[0].contents = &floppy;

	crates[1].exists = 1;
	crates[1].contents = &metal;

	MatchContext context = {
		.characters = {
			&hunters[0], &hunters[1],
			&hunters[2], &hunters[3],
		},
		.crates_len = 2,
		.crates = crates,
		.target_relic = &floppy,
		.scoring_context = &DEFAULT_SCORING_CONTEXT
	};

	decodeMap(&context,
			"   #C### ##\n"
			"# #H########\n"
			"#####     ##\n"
			" ##       ##\n"
			" ####     ##\n"
			"  #E#    #H#####\n"
			" ###     ###  ##\n"
			" ##H      ######\n"
			" ### ### ###\n"
			" #####C#H###\n"
			"  ### # ###\n"
		);

	initMatch(&context);
	
	int tryPath(int start_x, int start_y, int end_x, int end_y){
		PathNode * path = findPathWithin(
				&context,
				start_x, start_y,
				end_x, end_y,
				15
			);

		if (path == NULL) {
			printf("No path found :(\n\n");
			return 1;
		}

		// Print path
		for (PathNode * node = path; node; node = node->to)
			printf("%d, %d (%d)\n", node->x, node->y, node->distance);

		printf("\n");

		return 0;
	}

	tryPath(3,1, 15,6);
	tryPath(3,1, 11,2);
	// tryPath(3,1, 15,600);
	return 0;
}
