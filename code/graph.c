#include "graph.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>



// path_map either has to be a valid path map with the same dimensions as the graph, or NULL.
// If it is NULL, it will be ignored and the graph will be printed without marking the path.
void print_graph(Graph* const g, bool** path_map)
{
    const char* neighbor_symbol[] = {"·", "╶", "╴", "─", "╵", "└", "┘", "┴",
                                     "╷", "┌", "┐", "┬", "│", "├", "┤", "┼"};

    const char* cost_color[] = {"37", "1;32", "1;36", "1;33", "1;31"};
    assert(g->height <= INT16_MAX); // assert that it fits into an int16_t (line below)
    for(int16_t y = (int16_t)g->height - 1; y >= 0; y--) {
        for(uint16_t x = 0; x < g->width; x++) {
            bool is_start_or_end = (g->p1.x == x && g->p1.y == y) || (g->p2.x == x && g->p2.y == y);
            bool is_part_of_path = (path_map != NULL) && path_map[x][y];
            printf("\033[0;%s%s%sm%s", cost_color[g->node_cost[x][y]], is_start_or_end ? ";43" : "",
                   is_part_of_path ? ";5" : "", neighbor_symbol[g->neighbors[x][y]]);
        }
        printf("\033[0m\n");
    }
    printf("width = %" PRIu16 ", height = %" PRIu16 "\n", g->width, g->height);
    printf("Node cost (number of intersections): \033[0;%sm0\033[0m, \033[0;%sm1\033[0m, "
           "\033[0;%sm2\033[0m, \033[0;%sm3\033[0m, \033[0;%sm4\033[0m;\n",
           cost_color[0], cost_color[1], cost_color[2], cost_color[3], cost_color[4]);
    printf("\033[0;43m \033[0m = Points to connect\n\n");
}



// free everything inside the graph and the graph itself
void graph_free(Graph* const g)
{
    free(g->neighbors[0]);
    free(g->node_cost[0]);
    free(g->neighbors);
    g->neighbors = NULL;
    free(g->node_cost);
    g->node_cost = NULL;
    free(g);
}



Graph* graph_malloc(const long width, const long height)
{
    // if you change anything here, you may also need to adapt the function graph_free

    Graph* g = malloc(sizeof(Graph));
    if(!g) {
        fprintf(stderr, "Allocation for Graph failed.\n");
        exit(EXIT_FAILURE);
    }
    g->neighbors = malloc((unsigned long)width * sizeof(uint8_t*));
    g->node_cost = malloc((unsigned long)width * sizeof(uint8_t*));
    if(!g->neighbors || !g->node_cost) {
        fprintf(stderr, "Allocation for Graph (outer array) failed.\n");
        exit(EXIT_FAILURE);
    }
    g->neighbors[0] = malloc((unsigned long)(width * height) * sizeof(uint8_t));
    g->node_cost[0] = malloc((unsigned long)(width * height) * sizeof(uint8_t));
    if(!g->neighbors[0] || !g->node_cost[0]) {
        fprintf(stderr, "Allocation for Graph (inner array) failed.\n");
        exit(EXIT_FAILURE);
    }
    for(int x = 1; x < width; x++) {
        g->neighbors[x] = g->neighbors[0] + (x * height);
        g->node_cost[x] = g->node_cost[0] + (x * height);
    }
    return g;
}
