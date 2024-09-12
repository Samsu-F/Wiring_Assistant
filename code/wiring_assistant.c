#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <inttypes.h>

#include "endpoint_repr.h"
#include "graph.h"
#include "a_star.h"



// parse args and write 0 or 1 to given pointers
static bool parse_command_line_args(int argc, char** argv, int* gflag_ptr, int* hflag_ptr,
                                    int* pflag_ptr, int* tflag_ptr)
{
    *gflag_ptr = 0;
    *hflag_ptr = 0;
    *pflag_ptr = 0;
    *tflag_ptr = 0;

    opterr = 0;

    int c;
    while((c = getopt(argc, argv, "ghpt")) != -1)
        switch(c) {
            case 'g':
                *gflag_ptr = 1;
                break;
            case 'h':
                *hflag_ptr = 1;
                break;
            case 'p':
                *pflag_ptr = 1;
                *gflag_ptr = 1; // -p implies -g
                break;
            case 't':
                *tflag_ptr = 1;
                break;
            case '?':
                if(isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                return false;
            default:
                return false;
        }
    return true;
}



// Parse a problem instance read from stdin and write it into er.
// Argument er must not be a nullpointer.
// er->wires will be overwritten with NULL or a pointer to newly allocated memory, for which the
// caller is responsible for freeing.
static void parse_endpoint_repr(EndpointRepr* const er)
{
    assert(er);
    // read first line. Semantics: M S; Format ^[0-9]{1,2} [0-9]{1,9}$
    scanf("%d %" SCNdFAST32, &(er->m), &(er->width));
    er->height = er->width;
    if(er->width == 0) { // if the line just parsed marks the end of the input
        // ensure there is no random data there which might be falsely interpretet as a pointer
        er->wires = NULL;
        return;
    }
    er->wires = malloc(er->m * sizeof(Wire));
    if(!er->wires) {
        fprintf(stderr, "Allocating %lu bytes for EndpointRepr wires array failed.\n", er->m * sizeof(Wire));
        exit(EXIT_FAILURE);
    }

    // read second line. Semantics: (x_left y_bottom x_right y_bottom)*M; Format[0-9]{1,9} 4M times
    for(int i = 0; i < er->m; i++) {
        int_fast32_t x1, y1, x2, y2;
        scanf("%" SCNdFAST32 " %" SCNdFAST32 " %" SCNdFAST32 " %" SCNdFAST32, &x1, &y1, &x2, &y2);
        er->wires[i].x1 = x1;
        er->wires[i].y1 = y1;
        er->wires[i].x2 = x2;
        er->wires[i].y2 = y2;
    }

    // read third line. Semantics: p1_x p1_y p2_x p2_y; Format ^[0-9]{1,9} [0-9]{1,9} [0-9]{1,9} [0-9]{1,9}$
    scanf("%" SCNdFAST32 " %" SCNdFAST32 " %" SCNdFAST32 " %" SCNdFAST32, &(er->p1x), &(er->p1y),
          &(er->p2x), &(er->p2y));
}



// Build a graph based on er
// Caller is responsible for freeing returned graph with graph_free
static Graph* build_graph(const EndpointRepr* const er)
{
    Graph* g = graph_malloc(er->width, er->height);
    g->width = er->width;
    g->height = er->height;
    // check if width and height are not too small for new data type
    assert(((int_fast32_t)g->width == er->width) && ((int_fast32_t)g->height == er->height));
    g->p1 = (Uint16Point) {(uint16_t)er->p1x, (uint16_t)er->p1y};
    g->p2 = (Uint16Point) {(uint16_t)er->p2x, (uint16_t)er->p2y};
    // by default nodes have a cost of 0
    memset(g->node_cost[0], 0, er->width * er->height * sizeof(uint8_t));

    uint8_t bitmask_all_neighbors = NEIGH_X_NEG | NEIGH_X_POS | NEIGH_Y_NEG | NEIGH_Y_POS;
    memset(g->neighbors[0], bitmask_all_neighbors, er->width * er->height);
    // now every node is marked as having all four neighbors. Remove neighbors where this does not apply
    memset(g->neighbors[0], NEIGH_X_POS | NEIGH_Y_NEG | NEIGH_Y_POS, er->height);
    memset(g->neighbors[er->width - 1], NEIGH_X_NEG | NEIGH_Y_NEG | NEIGH_Y_POS, er->height);
    for(int_fast32_t x = 0; x < er->width; x++) {
        g->neighbors[x][0] &= ~NEIGH_Y_NEG; // unset bit indicating neighbor in negative y direction
        g->neighbors[x][er->height - 1] &= ~NEIGH_Y_POS;
    }
    // remove edges where existing wires are
    for(int i = 0; i < er->m; i++) {             // for each wire in er
        if(er->wires[i].y1 == er->wires[i].y2) { // horizontal wire in x direction
            int_fast32_t x1 = er->wires[i].x1;
            int_fast32_t x2 = er->wires[i].x2;
            int_fast32_t y = er->wires[i].y1;
            assert(x1 < x2);
            g->neighbors[x1][y] &= ~NEIGH_X_POS; // no neighbor in positive x direction
            g->node_cost[x1][y] += 1;            // increase cost
            for(int_fast32_t x = x1 + 1; x < x2; x++) {
                g->neighbors[x][y] &= ~(NEIGH_X_NEG | NEIGH_X_POS); // no neighbor in +- x direction
                g->node_cost[x][y] += 1;                            // increase cost
            }
            g->neighbors[x2][y] &= ~NEIGH_X_NEG; // no neighbor in negative x direction
            g->node_cost[x2][y] += 1;            // increase cost
        }
        else { // vertical wire in y direction. Basically the same procedure as for horizontal wires
            assert(er->wires[i].x1 == er->wires[i].x2);
            int_fast32_t y1 = er->wires[i].y1;
            int_fast32_t y2 = er->wires[i].y2;
            int_fast32_t x = er->wires[i].x1;
            assert(y1 < y2);
            g->neighbors[x][y1] &= ~NEIGH_Y_POS;
            g->node_cost[x][y1] += 1;
            for(int_fast32_t y = y1 + 1; y < y2; y++) {
                g->neighbors[x][y] &= ~(NEIGH_Y_NEG | NEIGH_Y_POS);
                g->node_cost[x][y] += 1;
            }
            g->neighbors[x][y2] &= ~NEIGH_Y_NEG;
            g->node_cost[x][y2] += 1;
        }
    }
    return g;
}



// calculate the absolute difference between two uint16 values
static inline uint16_t abs_diff(const uint16_t a, const uint16_t b)
{
    return a > b ? a - b : b - a;
}

// calculate the manhattan distance between two Uint16Point
// manhattan_distance(p1, p2) = manhattan_distance(p2, p1) >= 0
static uint16_t manhattan_distance(const Uint16Point p, const Uint16Point goal)
{
    return abs_diff(p.x, goal.x) + abs_diff(p.y, goal.y);
}



static void print_help(char* argv0)
{
    printf("Usage: %s [OPTIONS]\n", argv0);
    printf("Read input from the 1006 ACM ICPC Problem 'Wiring Assistant' from stdin and solve it.\n\n");
    printf("Options:\n");
    printf("  -h\t(help)\tShow this help message and exit.\n");
    printf("  -g\t(graph)\tPrint the graph after the reduction step.\n");
    printf("  -p\t(path)\tMark the cheapest path in the printed graph. Implies -g.\n");
    printf("  -t\t(time)\tMeasure and print the time to run each step.\n");
    printf("\nAuthor\n  Written by Samuel Füßinger, 2024.\n  github.com/Samsu-F\n"); // Author and License ?
}



int main(int argc, char** argv)
{
    int gflag, hflag, pflag, tflag; // command line flags for printing the graph and time
    if(!parse_command_line_args(argc, argv, &gflag, &hflag, &pflag, &tflag)) {
        fprintf(stderr, "Parsing command line args failed.\n");
        exit(EXIT_FAILURE);
    }
    if(hflag) {         // help flag was set, so print help and exit
        if(argv == 0) { // argc == 0 cannot happen with normal usage
            exit(EXIT_FAILURE);
        }
        print_help(argv[0]);
        exit(EXIT_SUCCESS);
    }

    while(true) {
        // Plan of attack:
        //    1. Parse one problem instance from stdin
        //    2. Reduction
        //    3. Build graph
        //    4. Calculate cost of cheapest path using A*
        //    5. Optionally print graph and stopwatch times, print result

        clock_t time_0 = clock();

        EndpointRepr endpoint_repr;
        parse_endpoint_repr(&endpoint_repr);
        clock_t time_1 = clock();

        if(endpoint_repr.width == 0) { // if end of input was reached
            return EXIT_SUCCESS;
        }

        reduce(&endpoint_repr);
        clock_t time_2 = clock();

        Graph* graph = build_graph(&endpoint_repr);
        clock_t time_3 = clock();

        int16_t minimal_intersections;
        bool** path_map = NULL; // only used if the -p flag is set
        if(pflag) {             // use a_star_path_map so that we can print the cheapest path later
            path_map = new_path_map(graph->width, graph->height);
            minimal_intersections = a_star_path_map(graph, manhattan_distance, path_map);
        }
        else { // use the marginally quicker version if the path will not be needed later
            minimal_intersections = a_star_cost(graph, manhattan_distance);
        }
        clock_t time_4 = clock();

        if(gflag) {
            print_graph(graph, path_map);
        }

        if(tflag) { // print stopwatch times
            float ms_parse_input = (float)(1000 * (time_1 - time_0)) / CLOCKS_PER_SEC;
            float ms_simplify = (float)(1000 * (time_2 - time_1)) / CLOCKS_PER_SEC;
            float ms_build_gr = (float)(1000 * (time_3 - time_2)) / CLOCKS_PER_SEC;
            float ms_min_inters = (float)(1000 * (time_4 - time_3)) / CLOCKS_PER_SEC;
            printf("parse input:    %7.3f ms\n"
                   "reduce:         %7.3f ms\n"
                   "build graph:    %7.3f ms\n"
                   "A*:             %7.3f ms\n",
                   ms_parse_input, ms_simplify, ms_build_gr, ms_min_inters);
        }

        printf("%d\n", minimal_intersections); // print result

        free(endpoint_repr.wires);
        endpoint_repr.wires = NULL;
        graph_free(graph);
        graph = NULL;
        if(pflag) {
            free_path_map(path_map);
        }
    }
}
