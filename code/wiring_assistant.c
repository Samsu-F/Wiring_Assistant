#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#define BLOOM1_SIZE 1019
#define BLOOM2_SIZE 1021
#define MAX_M 99            // 0 < m < 100
#define MAX_S 999999999     // 0 < s < 1000000000

typedef struct horizontal_cable {
    uint32_t y;
    uint32_t x1;    // x1 < x2
    uint32_t x2;
} horizontal_cable;

typedef struct vertical_cable {
    uint32_t x;
    uint32_t y1;    // y1 < y2
    uint32_t y2;
} vertical_cable;


typedef struct graph {
    uint m;     // number of existing cables, m <= 0 <= MAX_M
    uint32_t s; // size (s = length = width)
    uint32_t p1x, p1y, p2x, p2y;    // the points that have to be connected
    bool bloom1_x[BLOOM1_SIZE];
    bool bloom1_y[BLOOM1_SIZE];
    bool bloom2_x[BLOOM2_SIZE];
    bool bloom2_y[BLOOM2_SIZE];
    uint m_horizontal;                      // the number of horizontal cables
    horizontal_cable horizontal_cables[99]; // sorted by their y coordinate
    uint m_vertical;                        // the number of vertical cables
    vertical_cable vertical_cables[99];     // sorted by their x coordinate
} graph;


int compare_cables(const void* a, const void* b)
{
    vertical_cable* cable_a = (vertical_cable*)a; // TODO: is this fine to use for horizontal cables or is this undefined behavior?
    vertical_cable* cable_b = (vertical_cable*)b;
    if(cable_a->x < cable_b->x) {
        return -1;
    } else if(cable_a->x > cable_b->x) {
        return 1;
    } else {
        assert((cable_a->y2 <= cable_b->y1) || (cable_b->y2 <= cable_a->y1)); // assert they do not overlap
        if(cable_a->y1 < cable_b->y1) {
            return -1;
        }
        return 1;
    }
}


// parse stdin and create the graph struct.
// returns NULL if there is no next problem instance
graph* parse_instance()
{
    graph* const g = malloc(sizeof(graph));
    if(!g) {
        fprintf(stderr, "Allocating %ld bytes for graph struct failed.\n", sizeof(graph));
        return NULL;
    }
    g->m_horizontal = 0;
    g->m_vertical = 0;

    // read first line. Semantics: M S; Format ^[0-9]{1,2} [0-9]{1,9}$
    scanf("%u %u\n", &(g->m), &(g->s));
    fprintf(stderr, "DEBUG: g->m = %u, g->s = %u\n", g->m, g->s); ////////////////// debug
    if(!g->s) { exit(EXIT_SUCCESS); } // the line just parsed marks the end of the input // TODO: is this good style?

    // read second line. Semantics: (x_left y_bottom x_right y_bottom)*M; Format [0-9]{1,9} 4M times
    for(uint i = 0; i < g->m; i++) {
        uint x1, y1, x2, y2;
        scanf("%u %u %u %u", &x1, &y1, &x2, &y2);
        printf("DEBUG: x1=%u y1=%u x2=%u y2=%u\n", x1, y1, x2, y2); ////////////////////////////// debug
        if(x1 == x2) { // vertical cable
            assert(y1 < y2);
            g->vertical_cables[g->m_horizontal].x = x1;
            g->vertical_cables[g->m_horizontal].y1 = y1;
            g->vertical_cables[g->m_horizontal].y2 = y2;
            g->m_horizontal++;
        } else { // vertical cable
            assert(y1 == y2 && x1 < x2);
            g->horizontal_cables[g->m_vertical].y = y1;
            g->horizontal_cables[g->m_vertical].x1 = x1;
            g->horizontal_cables[g->m_vertical].x2 = x2;
            g->m_vertical++;
        }
    }
    // sort cables
    qsort(g->horizontal_cables, g->m_horizontal, sizeof(horizontal_cable), compare_cables);
    qsort(g->vertical_cables, g->m_vertical, sizeof(vertical_cable), compare_cables);

    // read third line. Semantics: p1_x p1_y p2_x p2_y; Format ^[0-9]{1,9} [0-9]{1,9} [0-9]{1,9} [0-9]{1,9}$
    scanf("%u %u %u %u", &(g->p1x), &(g->p1y), &(g->p2x), &(g->p2y));

    /* DEBUG*/ printf("HORIZONTAL:\n"); for(uint i = 0; i < g->m_horizontal; i++){ printf("\t(%u, %u) (%u, %u)\n", g->horizontal_cables[i].x1, g->horizontal_cables[i].y, g->horizontal_cables[i].x2, g->horizontal_cables[i].y);}printf("VERTICAL:\n");for(uint i = 0; i < g->m_vertical; i++){printf("\t(%u, %u) (%u, %u)\n", g->vertical_cables[i].x, g->vertical_cables[i].y1, g->vertical_cables[i].x, g->vertical_cables[i].y2);}
    return g;
}


int main(/*int argc, char* argv[]*/)
{
    while(1) {
        const graph* const g = parse_instance();
        if(!g) { return EXIT_FAILURE; }
    }
}
