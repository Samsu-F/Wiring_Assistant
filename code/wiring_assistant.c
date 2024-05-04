#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>



#define MAX_M 99        // 0 < m < 100
#define MAX_S 999999999 // 0 < s < 1000000000



typedef struct Wire {
    long x1;
    long y1;
    long x2;
    long y2;
} Wire;



typedef struct RawData {
    int m;       // number of wires
    long width;  // number of nodes in the x direction
    long height; // number of nodes in the y direction
    long p1x;    // coordinates of the start and end points
    long p1y;
    long p2x;
    long p2y;
    Wire* wires; // the given wire coordinates
} RawData;



typedef struct Graph {
    long p1x; // coordinates of the start and end points
    long p1y;
    long p2x;
    long p2y;
    uint8_t** node_cost;
    uint8_t** neighbors;
} Graph;

const uint8_t NEIGH_X_POS = 0x01;
const uint8_t NEIGH_X_NEG = 0x02;
const uint8_t NEIGH_Y_POS = 0x04;
const uint8_t NEIGH_Y_NEG = 0x08;



// Comparison function for qsort
int compare_long(const void* a, const void* b)
{
    const long x = *((const long*)a);
    const long y = *((const long*)b);
    return (x > y) - (x < y);
}



// TODO: remove?
void debug_print_raw_data(RawData* const rd)
{
    fprintf(stderr, "DEBUG: raw data\n");
    fprintf(stderr, "\tm = %d\n\twidth = %ld\n\theight = %ld\n", rd->m, rd->width, rd->height);
    fprintf(stderr, "\tp1 = (%ld, %ld)\n\tp2 = (%ld, %ld)\n", rd->p1x, rd->p1y, rd->p2x, rd->p2y);

    fprintf(stderr, "\tCABLES:\n");
    for(int i = 0; i < rd->m; i++) {
        Wire w = rd->wires[i];
        fprintf(stderr, "\t\t(%ld, %ld) <-> (%ld, %ld)\n", w.x1, w.y1, w.x2, w.y2);
    }
}



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



// helper function for simplify
// subtract shift from all x-coordinates of wire end points if >= min_to_move
void simplify_x_shift(RawData* const rd, const long shift, const long min_to_move)
{
    assert(shift > 0);
    rd->width -= shift;
    for(int i = 0; i < rd->m; i++) {
        if(rd->wires[i].x2 >= min_to_move) {
            rd->wires[i].x2 -= shift;
            // nested because x1 <= x2, so if x2 < min_to_move, then x1 will also be < min_to_move
            if(rd->wires[i].x1 >= min_to_move) {
                rd->wires[i].x1 -= shift;
            }
        }
    }
    if(rd->p1x >= min_to_move) {
        rd->p1x -= shift;
    }
    if(rd->p2x >= min_to_move) {
        rd->p2x -= shift;
    }
}
// analogous for y
// TODO: refactor and find a better way without repetition
void simplify_y_shift(RawData* const rd, const long shift, const long min_to_move)
{
    assert(shift > 0);
    rd->height -= shift;
    for(int i = 0; i < rd->m; i++) {
        if(rd->wires[i].y2 >= min_to_move) {
            rd->wires[i].y2 -= shift;
            if(rd->wires[i].y1 >= min_to_move) {
                rd->wires[i].y1 -= shift;
            }
        }
    }
    if(rd->p1y >= min_to_move) {
        rd->p1y -= shift;
    }
    if(rd->p2y >= min_to_move) {
        rd->p2y -= shift;
    }
}



// TODO: refactor
// Simplify the grid by removing identical neighboring columns/rows.
// For number of existing wires m, after this function both the width and the height of the grid are
// guaranteed to be equal to or less than 2*m+5. Since there can only be at most 3 unique
// coordinates per cable, their sum is guaranteed to be <= 2*(2*m+5)-m = 3*m+10.
// Therefore, their product (= total number of nodes) is <= ((3*m+10)/2)^2 = (1.5*m+5)^2
void simplify(RawData* const rd)
{
    assert(rd != NULL && rd->m > 0 && rd->wires != NULL);
    size_t n = 2 * rd->m + 3; // the number of coordinates per direction

    long xs[n]; // worst case size for these two VLAs is 1608 bytes each,
    long ys[n]; // assuming sizeof(long) = 8. So unless ran on a very limited embedded system,
                // the max stack size will definitely not be a problem.
    for(int i = 0; i < rd->m; i++) {
        xs[2 * i] = rd->wires[i].x1;
        ys[2 * i] = rd->wires[i].y1;
        xs[2 * i + 1] = rd->wires[i].x2;
        ys[2 * i + 1] = rd->wires[i].y2;
    }
    xs[n - 3] = rd->width;
    ys[n - 3] = rd->height;
    xs[n - 2] = rd->p1x;
    ys[n - 2] = rd->p1y;
    xs[n - 1] = rd->p2x;
    ys[n - 1] = rd->p2y;
    qsort(&xs, n, sizeof(long), compare_long);
    qsort(&ys, n, sizeof(long), compare_long);

    long prev = -1; // the coordinate of the closest left to it (the previous in this sorted array)
    long sum_shifts = 0; // to avoid having to update the rest of this sorted array each time,
                         // keep track of all the shifts so for
    for(size_t i = 0; i < n; i++) {
        xs[i] -= sum_shifts;
        long diff = xs[i] - prev;
        if(diff >= 3) {
            long shift = diff - 2;
            simplify_x_shift(rd, shift, xs[i]);
            sum_shifts += shift;
            xs[i] -= shift;
        }
        prev = xs[i];
    }
    // now the same for y
    prev = -1;
    sum_shifts = 0;
    for(size_t i = 0; i < n; i++) {
        ys[i] -= sum_shifts;
        long diff = ys[i] - prev;
        if(diff >= 3) {
            long shift = diff - 2;
            simplify_y_shift(rd, shift, ys[i]);
            sum_shifts += shift;
            ys[i] -= shift;
        }
        prev = ys[i];
    }

    //////// DEBUG //////// TODO: remove
    debug_print_raw_data(rd);
}



// helper function for build_graph
// it should probably not be called from anywhere else
void graph_malloc(Graph* const g, const RawData* const rd)
{
    g->neighbors = malloc(rd->width * sizeof(uint8_t*));
    if(!g->neighbors) {
        fprintf(stderr, "Allocating %lu bytes for Graph neighbors array of arrays failed.\n",
                rd->width * sizeof(uint8_t*));
        exit(EXIT_FAILURE);
    }
    g->neighbors[0] = malloc(rd->width * rd->height * sizeof(uint8_t));
    if(!g->neighbors[0]) {
        fprintf(stderr, "Allocating %lu bytes for Graph neighbors failed.\n",
                rd->width * rd->height * sizeof(uint8_t));
        exit(EXIT_FAILURE);
    }
    g->node_cost = malloc(rd->width * sizeof(uint8_t*));
    if(!g->node_cost) {
        fprintf(stderr, "Allocating %lu bytes for Graph node_cost array of arrays failed.\n",
                rd->width * sizeof(uint8_t*));
        exit(EXIT_FAILURE);
    }
    g->node_cost[0] = malloc(rd->width * rd->height * sizeof(uint8_t));
    if(!g->node_cost[0]) {
        fprintf(stderr, "Allocating %lu bytes for Graph node_cost failed.\n",
                rd->width * rd->height * sizeof(uint8_t));
        exit(EXIT_FAILURE);
    }
    for(int x = 1; x < rd->width; x++) {
        g->neighbors[x] = g->neighbors[0] + (x * rd->height);
        g->node_cost[x] = g->node_cost[0] + (x * rd->height);
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



// The pointers stored in g will be overwritten, so calling graph_free(g) before calling
// build_graph to overwrite an existing graph will be necessary in most cases to avoid a memory leak.
// Must not be called on a RawData struct without simplifying it first with simplify(rd)
void build_graph(Graph* const g, const RawData* const rd)
{
    graph_malloc(g, rd);
    g->p1x = rd->p1x;
    g->p1y = rd->p1y;
    g->p2x = rd->p2x;
    g->p2y = rd->p2y;
    memset(g->node_cost[0], 0, rd->width * rd->height); // by default nodes have a cost of 0

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
}



// TODO: explanation
int main(void)
{
    while(true) {
        RawData raw_data;
        read_raw_data(&raw_data);
        if(raw_data.width == 0) {
            return EXIT_SUCCESS; // end of input was reached
        }

        simplify(&raw_data);

        Graph graph;
        build_graph(&graph, &raw_data);

        // TODO: find path

        // TODO: print result

        free(raw_data.wires);
        graph_free(&graph);
    }
}
