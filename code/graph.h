#ifndef _GRAPH_H
#define _GRAPH_H



#include <stdint.h>
#include <stdbool.h>



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
    Uint16Point** previous; // the table to save the predecessor node for path finding to reconstruct the path
} Graph;

// the bitmasks to select the single boolean bits
#define NEIGH_X_POS 0x01
#define NEIGH_X_NEG 0x02
#define NEIGH_Y_POS 0x04
#define NEIGH_Y_NEG 0x08



void print_graph(Graph* const g, bool mark_path);


// allocate memory for a graph, all of its internal arrays and set their pointers correctly.
// guaranteed to return a valid pointer
Graph* graph_malloc(const long width, const long height);

// free all internal arrays and the graph itself. Must only be used if graph_malloc was used for
// allocation, otherwise there could be a memory leak.
void graph_free(Graph* const g);



#endif
