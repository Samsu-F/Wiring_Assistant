#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#define BLOOM1_SIZE 1019    // 1019 is prime
#define BLOOM2_SIZE 1021    // 1021 is prime
#define MAX_M 99            // 0 < m < 100
#define MAX_S 999999999     // 0 < s < 1000000000

typedef struct cable {
    // for horizontal cables, shared is the y coordinate, min is the left x coordinate and max is the right coordinate
    // analogous for vertical cables, shared=x, min=y1, max=y2
    uint32_t shared;    // the shared coordinate of both end points
    uint32_t min;       // min < max
    uint32_t max;
} cable;


typedef struct graph {
    uint m;     // number of existing cables, m <= 0 <= MAX_M
    uint32_t s; // size (s = length = width)
    uint32_t p1x, p1y, p2x, p2y;    // the points that have to be connected
    bool bloom1_horizontal[BLOOM1_SIZE];    // bloom filter 1 for horizontal cables
    bool bloom1_vertical[BLOOM1_SIZE];      // bloom filter 1 for vertical cables
    bool bloom2_horizontal[BLOOM2_SIZE];    // bloom filter 2 for horizontal cables
    bool bloom2_vertical[BLOOM2_SIZE];      // bloom filter 2 for vertical cables
    uint m_horizontal;                      // the number of horizontal cables
    cable horizontal_cables[MAX_M];         // array of horizontal cables, sorted by their y coordinate
    uint m_vertical;                        // the number of vertical cables
    cable* vertical_cables;                 // array of vertical cables, sorted by their x coordinate
} graph;


inline uint hash_bloom1(uint32_t key)
{
    return key % BLOOM1_SIZE;
}

inline uint hash_bloom2(uint32_t key)
{
    return key % BLOOM2_SIZE;
}


int compare_cables(const void* a, const void* b)
{
    cable* cable_a = (cable*)a;
    cable* cable_b = (cable*)b;
    if(cable_a->shared < cable_b->shared) {
        return -1;
    } else if(cable_a->shared > cable_b->shared) {
        return 1;
    } else {
        assert((cable_a->max <= cable_b->min) || (cable_b->max <= cable_a->min)); // assert they do not overlap
        if(cable_a->min < cable_b->min) {
            return -1;
        }
        return 1;
    }
}


void insert_cable(graph* const g, const uint x1, const uint y1, const uint x2, const uint y2)
{
    if(x1 == x2) {  // vertical cable
        assert(y1 < y2);
        g->horizontal_cables[g->m_horizontal].shared = x1;
        g->horizontal_cables[g->m_horizontal].min = y1;
        g->horizontal_cables[g->m_horizontal].max = y2;
        g->m_horizontal++;      // increment the counter
    } else {        // horizontal cable
        assert(y1 == y2 && x1 < x2);
        g->m_vertical++;        // increment the counter
        // extend the array by one to the negative direction, taking space from g->horizontal_cables
        g->vertical_cables--;
        g->vertical_cables[0].shared = y1;
        g->vertical_cables[0].min = x1;
        g->vertical_cables[0].max = x2;
    }
}


void init_bloom_filters(graph* const g)
{
    // content of bloom filters is undefined, so clear the bloom filters first
    memset(g->bloom1_horizontal, false, BLOOM1_SIZE * sizeof(bool));
    memset(g->bloom2_horizontal, false, BLOOM2_SIZE * sizeof(bool));
    memset(g->bloom1_vertical, false, BLOOM1_SIZE * sizeof(bool));
    memset(g->bloom2_vertical, false, BLOOM2_SIZE * sizeof(bool));

    for(uint i = 0; i < g->m_horizontal; i++) {
        uint32_t key = g->horizontal_cables[i].shared;
        g->bloom1_horizontal[hash_bloom1(key)] = true;
        g->bloom2_horizontal[hash_bloom2(key)] = true;
    }
    for(uint i = 0; i < g->m_vertical; i++) {
        uint32_t key = g->vertical_cables[i].shared;
        g->bloom1_vertical[hash_bloom1(key)] = true;
        g->bloom2_vertical[hash_bloom2(key)] = true;
    }
}


// parse stdin and create the graph struct.
// returns a graph with s==0 iff there is no next problem instance
graph* parse_instance()
{
    graph* const g = malloc(sizeof(graph));
    if(!g) {
        fprintf(stderr, "Allocating %ld bytes for graph struct failed.\n", sizeof(graph));
        return NULL;
    }
    g->m_horizontal = 0;
    g->m_vertical = 0;
    g->vertical_cables = &(g->horizontal_cables[MAX_M]);

    // read first line. Semantics: M S; Format ^[0-9]{1,2} [0-9]{1,9}$
    scanf("%u %u\n", &(g->m), &(g->s));
    fprintf(stderr, "\nDEBUG:\tM = %u; S = %u\n", g->m, g->s); // DEBUG
    if(!g->s) { return g; } // the line just parsed marks the end of the input

    // read second line. Semantics: (x_left y_bottom x_right y_bottom)*M; Format [0-9]{1,9} 4M times
    for(uint i = 0; i < g->m; i++) {
        uint x1, y1, x2, y2;
        scanf("%u %u %u %u", &x1, &y1, &x2, &y2);
        insert_cable(g, x1, y1, x2, y2);
    }
    // sort cables and init bloom filters
    qsort(g->horizontal_cables, g->m_horizontal, sizeof(cable), compare_cables);
    qsort(g->vertical_cables, g->m_vertical, sizeof(cable), compare_cables);
    init_bloom_filters(g);

    // read third line. Semantics: p1_x p1_y p2_x p2_y; Format ^[0-9]{1,9} [0-9]{1,9} [0-9]{1,9} [0-9]{1,9}$
    scanf("%u %u %u %u", &(g->p1x), &(g->p1y), &(g->p2x), &(g->p2y));

    /* DEBUG*/ fprintf(stderr, "HORIZONTAL:\n"); for(uint i = 0; i < g->m_horizontal; i++){ fprintf(stderr, "\t(%u, %u) (%u, %u)\n", g->horizontal_cables[i].min, g->horizontal_cables[i].shared, g->horizontal_cables[i].max, g->horizontal_cables[i].shared);} fprintf(stderr, "VERTICAL:\n");for(uint i = 0; i < g->m_vertical; i++){ fprintf(stderr, "\t(%u, %u) (%u, %u)\n", g->vertical_cables[i].shared, g->vertical_cables[i].min, g->vertical_cables[i].shared, g->vertical_cables[i].max);}
    return g;
}


int main()
{
    while(true) {
        const graph* const g = parse_instance();
        if(!g) { return EXIT_FAILURE; }     // although this should never occur, make sure g is not a nullpointer
        if(!g->s) { return EXIT_SUCCESS; }  // end of input
    }
}
