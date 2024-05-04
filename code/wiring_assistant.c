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



// Comparison function for qsort
int compare_long(const void* a, const void* b)
{
    long x = *((long*)a);
    long y = *((long*)b);
    return (x > y) - (x < y);
}



void debug_print_raw_data(RawData* const rd)
{
    fprintf(stderr, "DEBUG: raw data\n");
    fprintf(stderr, "\tm = %d\n\twidth = %lu\n\theight = %lu\n", rd->m, rd->width, rd->height);
    fprintf(stderr, "\tp1 = (%lu, %lu)\n\tp2 = (%lu, %lu)\n", rd->p1x, rd->p1y, rd->p2x, rd->p2y);

    fprintf(stderr, "\tCABLES:\n");
    for(int i = 0; i < rd->m; i++) {
        Wire w = rd->wires[i];
        fprintf(stderr, "\t\t(%lu, %lu) <-> (%lu, %lu)\n", w.x1, w.y1, w.x2, w.y2);
    }
}



// TODO: explanation
void read_raw_data(RawData* const rd)
{
    // read first line. Semantics: M S; Format ^[0-9]{1,2} [0-9]{1,9}$
    scanf("%d %lu", &(rd->m), &(rd->width));
    rd->height = rd->width;
    if(rd->width == 0) { // if the line just parsed marks the end of the input
        // ensure there is no random data there which might be falsely interpretet as a pointer
        rd->wires = NULL;
        return;
    }
    rd->wires = malloc(rd->m * sizeof(Wire)); // TODO: is this a good idea?
    if(!rd->wires) {
        fprintf(stderr, "Allocating %ld bytes for RawData struct failed.\n", rd->m * sizeof(Wire));
        exit(EXIT_FAILURE);
    }

    // read second line. Semantics: (x_left y_bottom x_right y_bottom)*M; Format[0-9]{1,9} 4M times
    for(int i = 0; i < rd->m; i++) {
        long x1, y1, x2, y2;
        scanf("%lu %lu %lu %lu", &x1, &y1, &x2, &y2);
        rd->wires[i].x1 = x1;
        rd->wires[i].y1 = y1;
        rd->wires[i].x2 = x2;
        rd->wires[i].y2 = y2;
    }

    // read third line. Semantics: p1_x p1_y p2_x p2_y; Format ^[0-9]{1,9} [0-9]{1,9} [0-9]{1,9} [0-9]{1,9}$
    scanf("%lu %lu %lu %lu", &(rd->p1x), &(rd->p1y), &(rd->p2x), &(rd->p2y));
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



// TODO: explanation
// TODO: refactor
void simplify(RawData* const rd)
{
    assert(rd->m > 0 && rd->wires != NULL);
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
            simplify_x_shift(rd, diff - 2, xs[i]);
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
            simplify_y_shift(rd, diff - 2, ys[i]);
        }
        prev = ys[i];
    }

    //////// DEBUG /////////////
    debug_print_raw_data(rd);
}



// TODO: explanation
int main()
{
    while(true) {
        RawData raw_data;
        read_raw_data(&raw_data);
        if(raw_data.width == 0) {
            return EXIT_SUCCESS; // end of input was reached
        }
        simplify(&raw_data);

        // TODO: transform the raw data into a data structure on which the pathfinding algorithm
        //       can work efficiently

        // TODO: find path

        // TODO: print result

        free(raw_data.wires);
    }
}
