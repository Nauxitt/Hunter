#include <stdlib.h>
#include <stdio.h>
#include "path.h"
#include "hunter.h"

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

	// Clear map pointer registers
	for (int n =0; n < context.map_w * context.map_h; n++)
		context.map[n].reg_ptr = NULL;

	int pointValid(int x, int y) {
		if ((x < 0) || (y < 0))
			return 0;

		if ((x >= context.map_w) || (y >= context.map_h))
			return 0;

		Tile * tile = &context.map[context.map_w * y + x];
		
		if (tile->exists == 0)
			return 0;
		
		// Don't re-scan tiles
		if (tile->reg_ptr)
			return 0;

		if (tile->hunter)
			return 0;

		return 1;
	}
	
	int start_x = 3, start_y = 1;
	int end_x = 15, end_y = 6;

	struct pathNode {
		int x, y;
		uint32_t distance;

		struct pathNode * from;
		struct pathNode * to;   // Next node in path, set when path is found.
		
		// Linked list for path endpoints. NULL if not an endpoint
		struct pathNode * prev_path;
		struct pathNode * next_path;
	};

	struct pathNode nodes[1000];
	int nodes_length = 0;

	struct pathNode * newNode() {
		return &nodes[nodes_length++];
	}

	struct pathNode * addNode(int x, int y, struct pathNode * from) {
		struct pathNode * node = newNode();
		node->x = x;
		node->y = y;

		// Set map-tile pointer register, allowing us to later check
		// if a position-indexed tile has a respective pathNode,
		// ensuring nodes are only scaned once.
		context.map[context.map_w * y + x].reg_ptr = node;

		node->distance = 0;
		if (from) {
			node->from = from;
			node->distance = from->distance + 1;
		}
		return node;
	}

	int inPath(struct pathNode * endpoint, int x, int y ) {
		// If we've recursed past the start of the path
		if (endpoint == NULL)
			return 0;

		// Check point position
		if ((endpoint->x == x) && (endpoint->y == y))
			return 1;

		// Recurse down path
		return inPath(endpoint->from, x, y);
	}

	struct pathNode * insertPath(struct pathNode * insert_point, struct pathNode * inserted) {
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

	struct pathNode * removePath(struct pathNode * path) {
		struct pathNode * insert_point = path->next_path;

		path->next_path->prev_path = path->prev_path;
		path->prev_path->next_path = path->next_path;
		
		path->next_path = NULL;
		path->prev_path = NULL;

		return insert_point;
	}

	struct pathNode * start_node = addNode(start_x, start_y, NULL);
	insertPath(NULL, start_node);
	struct pathNode * path_head = start_node;

	struct pathNode * final_path = NULL;

	int iterations = 0;
	while (path_head) {
		iterations++;
		int x = path_head->x, y = path_head->y;

		// If we've found the destination, nice, exit!
		if ((x == end_x) && (y == end_y)) {
			printf("Found a path at %d iterations!\n", iterations);
			final_path = path_head;
			break;
		}

		inline void expandPath(int x, int y) {
			if (!pointValid(x, y))
				return;
			
			if (inPath(path_head, x, y))
				return;

			insertPath(path_head->prev_path, addNode(x, y, path_head));
		}

		// Expand paths clockwise, starting up
		expandPath(x  , y-1);
		expandPath(x+1, y  );
		expandPath(x  , y+1);
		expandPath(x-1, y  );
		
		// Exit if we're at the last endpoint
		if (path_head->next_path == path_head) {
			printf("Hit last endpoint, exiting\n");
			break;
		}

		// Remove the old path and go to the next
		path_head = removePath(path_head);
	}

	// Write forward-path from backward-path,
	// set first node to the start of the path
	for (; final_path && final_path->from; final_path = final_path->from){
		final_path->from->to = final_path;
	}

	// Print path
	for (struct pathNode * node = final_path; node; node = node->to)
		printf("%d, %d\n", node->x, node->y);

	return 0;
}
