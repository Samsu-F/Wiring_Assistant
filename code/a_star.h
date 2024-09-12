#ifndef _A_STAR_H
#define _A_STAR_H



#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "graph.h"


// type definition for the type of heuristic fuctions
typedef uint16_t (*HeuristicFunc)(const Uint16Point p, const Uint16Point goal);

// Calculate the minimal cost possible for a path between p1 and p2, where the cost of a path is
// defined as the sum of the node costs of all the nodes in the path, including start and end.
int16_t a_star_cost(const Graph* const g, HeuristicFunc h);
// same as a_star_cost, but mark the cheapest path in the path map. Caller is responsible for
// giving an appropriate path map [also see new_path_map].
int16_t a_star_path_map(const Graph* const g, HeuristicFunc h, bool** path_map);


// Allocate and initialize a path map for the A* algorithm.
// May return NULL if allocation failed.
// Caller is responsible for freeing it with free_path_map.
bool** new_path_map(const size_t width, const size_t height);
// free a path map that was allocated by new_path_map
void free_path_map(bool** path_map);



#endif
