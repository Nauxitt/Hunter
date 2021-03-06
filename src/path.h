#ifndef __PATH_H
#define __PATH_H

#include <stdlib.h>
#include <stdint.h>

typedef struct _PathNode PathNode;
typedef struct _MatchContext MatchContext;
typedef struct _Tile Tile;

typedef struct _PathNode {
	int32_t x, y;
	int32_t distance;
	uint8_t scanned;

	MatchContext * context;

	PathNode * from;
	PathNode * to;   // Next node in path, set when path is found.
	
	// Linked list for path endpoints. NULL if not an endpoint
	PathNode * prev_path;
	PathNode * next_path;

	Tile * tile;
} PathNode;

PathNode * mapAddPathNode(MatchContext * context, PathNode * from, int x, int y);
void mapResetPathData(MatchContext * context);
void mapClearPathList(MatchContext * context);
PathNode * insertPathCircular(PathNode * insert_point, PathNode * inserted);
PathNode * insertPath(PathNode * insert_point, PathNode * inserted);
int inPath(PathNode * path, int x, int y );
int inPathFrom(PathNode * endpoint, int x, int y );
PathNode * removePath(PathNode * path);
PathNode * pathEndpoint(PathNode * path);
void freePath(PathNode * path);
int pathfindingMain();

/*
   Generates all paths within a certain radius, or generates all shortests paths possible within the map, starting from a certain point
*/
PathNode * generatePaths(MatchContext * context, int s_x, int s_y, int range);
PathNode * generatePathsWithin(MatchContext * context, int s_x, int s_y, int range);

/*
   Finds a path between two points.  Returns NULL if no path is found.
   If findPathWithin, only return a path within a certain specified distance.
*/
PathNode * findPathWithin(MatchContext * context, int s_x, int s_y, int e_x, int e_y, int distance);
PathNode * findPath(MatchContext * context, int s_x, int s_y, int e_x, int e_y);

#endif
