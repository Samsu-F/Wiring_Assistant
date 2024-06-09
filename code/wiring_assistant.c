#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <time.h> // DEBUG


#include "coordinate_struct.h"
#include "graph.h"
#include "a_star.h"

/// TODO: split into multiple files
/// TODO: refactor nearly everything



// #define MAX_M 99        // 0 < m < 100
// #define MAX_S 999999999 // 0 < s < 1000000000


/// TODO: find better solution for this
// #define PRINT_DEBUG     false
// #define PRINT_GRAPHS    false
#define PRINT_STOPWATCH true



// TODO: explanation
void read_raw_data(RawData* const rd)
{
    // read first line. Semantics: M S; Format ^[0-9]{1,2} [0-9]{1,9}$
    scanf("%d %ld", &(rd->m), &(rd->width));
    rd->height = rd->width;
    if(rd->width == 0) { // if the line just parsed marks the end of the input
        // ensure there is no random data there which might be falsely interpretet as a pointer
        rd->wires = NULL;
        return;
    }
    rd->wires = malloc(rd->m * sizeof(Wire)); // TODO: is this a good idea?
    if(!rd->wires) {
        fprintf(stderr, "Allocating %lu bytes for RawData wires array failed.\n", rd->m * sizeof(Wire));
        exit(EXIT_FAILURE);
    }

    // read second line. Semantics: (x_left y_bottom x_right y_bottom)*M; Format[0-9]{1,9} 4M times
    for(int i = 0; i < rd->m; i++) {
        long x1, y1, x2, y2;
        scanf("%ld %ld %ld %ld", &x1, &y1, &x2, &y2);
        rd->wires[i].x1 = x1;
        rd->wires[i].y1 = y1;
        rd->wires[i].x2 = x2;
        rd->wires[i].y2 = y2;
    }

    // read third line. Semantics: p1_x p1_y p2_x p2_y; Format ^[0-9]{1,9} [0-9]{1,9} [0-9]{1,9} [0-9]{1,9}$
    scanf("%ld %ld %ld %ld", &(rd->p1x), &(rd->p1y), &(rd->p2x), &(rd->p2y));
}



// TODO: split into two parts, one of which is independent from coordinate_struct
// Build a graph based on rd
// Caller is responsible for freeing returned graph with graph_free
Graph* build_graph(const RawData* const rd)
{
    Graph* g = graph_malloc(rd->width, rd->height);
    g->width = rd->width;
    g->height = rd->height;
    g->p1 = (Uint16Point) {(uint16_t)rd->p1x, (uint16_t)rd->p1y};
    g->p2 = (Uint16Point) {(uint16_t)rd->p2x, (uint16_t)rd->p2y};
    // by default nodes have a cost of 0
    memset(g->node_cost[0], 0, rd->width * rd->height * sizeof(uint8_t));

    uint8_t bitmask_all_neighbors = NEIGH_X_NEG | NEIGH_X_POS | NEIGH_Y_NEG | NEIGH_Y_POS;
    memset(g->neighbors[0], bitmask_all_neighbors, rd->width * rd->height);
    // now every node is marked as having all four neighbors. Remove neighbors where this does not apply
    memset(g->neighbors[0], NEIGH_X_POS | NEIGH_Y_NEG | NEIGH_Y_POS, rd->height);
    memset(g->neighbors[rd->width - 1], NEIGH_X_NEG | NEIGH_Y_NEG | NEIGH_Y_POS, rd->height);
    for(long x = 0; x < rd->width; x++) {
        g->neighbors[x][0] &= ~NEIGH_Y_NEG; // unset bit indicating neighbor in negative y direction
        g->neighbors[x][rd->height - 1] &= ~NEIGH_Y_POS;
    }
    for(int i = 0; i < rd->m; i++) {             // for each wire in rd
        if(rd->wires[i].y1 == rd->wires[i].y2) { // horizontal wire in x direction
            long x1 = rd->wires[i].x1;
            long x2 = rd->wires[i].x2;
            long y = rd->wires[i].y1;
            assert(x1 < x2);
            g->neighbors[x1][y] &= ~NEIGH_X_POS; // no neighbor in positive x direction
            g->node_cost[x1][y] += 1;            // increase cost
            for(long x = x1 + 1; x < x2; x++) {
                g->neighbors[x][y] &= ~(NEIGH_X_NEG | NEIGH_X_POS); // no neighbor in +- x direction
                g->node_cost[x][y] += 1;                            // increase cost
            }
            g->neighbors[x2][y] &= ~NEIGH_X_NEG; // no neighbor in positive x direction
            g->node_cost[x2][y] += 1;            // increase cost
        }
        // TODO: refactor and find better way to do this without repetition
        else { // vertical wire in y direction. Basically the same procedure as for horizontal wires
            assert(rd->wires[i].x1 == rd->wires[i].x2);
            long y1 = rd->wires[i].y1;
            long y2 = rd->wires[i].y2;
            long x = rd->wires[i].x1;
            assert(y1 < y2);
            g->neighbors[x][y1] &= ~NEIGH_Y_POS;
            g->node_cost[x][y1] += 1;
            for(long y = y1 + 1; y < y2; y++) {
                g->neighbors[x][y] &= ~(NEIGH_Y_NEG | NEIGH_Y_POS);
                g->node_cost[x][y] += 1;
            }
            g->neighbors[x][y2] &= ~NEIGH_Y_NEG;
            g->node_cost[x][y2] += 1;
        }
    }
    return g;
}



static inline uint16_t abs_diff(const uint16_t a, const uint16_t b)
{
    return a > b ? a - b : b - a;
}
uint16_t manhattan_distance(const Uint16Point p, const Uint16Point goal)
{
    return abs_diff(p.x, goal.x) + abs_diff(p.y, goal.y);
}



// TODO: explanation
int main(void)
{
    while(true) {
        clock_t time_0 = clock();

        RawData raw_data;
        read_raw_data(&raw_data);
        if(raw_data.width == 0) {
            return EXIT_SUCCESS; // end of input was reached
        }
        clock_t time_1 = clock();

        reduce(&raw_data);
        clock_t time_2 = clock();

        Graph* graph_p = build_graph(&raw_data);
        clock_t time_3 = clock();

        debug_print_graph(graph_p);
        clock_t time_4 = clock();

        int minimal_intersections = a_star_cost(graph_p, manhattan_distance);
        clock_t time_5 = clock();

        printf("%d\n", minimal_intersections);

        free(raw_data.wires);
        raw_data.wires = NULL;
        graph_free(graph_p);
        graph_p = NULL;

        if(PRINT_STOPWATCH) {
            float ms_read_rd = (float)(1000 * (time_1 - time_0)) / CLOCKS_PER_SEC;
            float ms_simplify = (float)(1000 * (time_2 - time_1)) / CLOCKS_PER_SEC;
            float ms_build_gr = (float)(1000 * (time_3 - time_2)) / CLOCKS_PER_SEC;
            float ms_min_inters = (float)(1000 * (time_5 - time_4)) / CLOCKS_PER_SEC;
            printf("read_raw_data:         %7.3f ms\n"
                   "simplify:              %7.3f ms\n"
                   "build_graph:           %7.3f ms\n"
                   "minimal_intersections: %7.3f ms\n\n",
                   ms_read_rd, ms_simplify, ms_build_gr, ms_min_inters);
        }
    }
}
