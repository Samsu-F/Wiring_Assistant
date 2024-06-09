#include "graph.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>



/// TODO: find better solution for this
// #define PRINT_DEBUG     false
#define PRINT_GRAPHS false
// #define PRINT_STOPWATCH true



// TODO: remove?
void debug_print_graph(Graph* const g)
{
    if(PRINT_GRAPHS) {
        const char* neighbor_symbol[] = {"·", "╶", "╴", "─", "╵", "└", "┘", "┴",
                                         "╷", "┌", "┐", "┬", "│", "├", "┤", "┼"};

        const char* cost_color[] = {"\033[0;37m", "\033[1;32m", "\033[1;36m", "\033[1;33m", "\033[1;31m"};
        for(int16_t y = g->height - 1; y >= 0; y--) {
            for(uint16_t x = 0; x < g->width; x++) {
                printf("%s%s", cost_color[g->node_cost[x][y]], neighbor_symbol[g->neighbors[x][y]]);
            }
            printf("\033[0m\n");
        }
        printf("Color indicates the cost (number of intersections) of a node: %s0, %s1, %s2, %s3, %s4\033[0m\n",
               cost_color[0], cost_color[1], cost_color[2], cost_color[3], cost_color[4]);
    }
}



// free everything inside the Graph struct
void graph_free(Graph* const g)
{
    free(g->neighbors[0]);
    free(g->node_cost[0]);
    free(g->neighbors);
    g->neighbors = NULL;
    free(g->node_cost);
    g->node_cost = NULL;
}
