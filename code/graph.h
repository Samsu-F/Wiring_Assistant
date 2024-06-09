#ifndef _GRAPH_H
#define _GRAPH_H



#include <stdint.h>



typedef struct PathCost {
    uint8_t intersections;
    uint16_t length;
} PathCost;

typedef struct Uint16Point {
    uint16_t x;
    uint16_t y;
} Uint16Point;

typedef struct Graph {
    uint16_t width;
    uint16_t height;
    Uint16Point p1; // coordinates of the start and end points
    Uint16Point p2;
    uint8_t** node_cost;
    uint8_t** neighbors;
} Graph;

// the bitmasks to select the single boolean bits
#define NEIGH_X_POS 0x01
#define NEIGH_X_NEG 0x02
#define NEIGH_Y_POS 0x04
#define NEIGH_Y_NEG 0x08



void debug_print_graph(Graph* const g);

void graph_free(Graph* const g);



#endif
