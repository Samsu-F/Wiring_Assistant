#ifndef _A_STAR_H
#define _A_STAR_H



#include <stdint.h>

#include "graph.h"


// type definition for the type of heuristic fuctions
typedef uint16_t (*HeuristicFunc)(const Uint16Point p, const Uint16Point goal);

int16_t a_star_cost(const Graph* const g, HeuristicFunc h);



#endif
