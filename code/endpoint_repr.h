#ifndef _ENDPOINT_REPR_H
#define _ENDPOINT_REPR_H



#include <stddef.h>
#include <stdint.h>



// Representation of a wire by the coordinates of its end points
// It must hold that x1 <= x2 and y1 <= y2
typedef struct Wire {
    int_fast32_t x1;
    int_fast32_t y1;
    int_fast32_t x2;
    int_fast32_t y2;
} Wire;



// Representation of a problem instance by the coordinates of the endpoints of its wires.
// This form of representaion is useful for parsing and reduction but it is not intended for
// running a pathfinding algorithm on it.
typedef struct EndpointRepr {
    int m;               // number of wires
    int_fast32_t width;  // number of nodes in the x direction
    int_fast32_t height; // number of nodes in the y direction
    int_fast32_t p1x;    // coordinates of the start and end points
    int_fast32_t p1y;
    int_fast32_t p2x;
    int_fast32_t p2y;
    Wire* wires; // the given wire coordinates
} EndpointRepr;



// Simplify the grid by removing identical neighboring columns/rows.
// Must not be called on a nullpointer.
// For number of existing wires m, after this function both the width and the height of the grid are
// guaranteed to be equal to or less than 4*m+5. Since there can only be at most 3 unique
// coordinates per cable, their sum is guaranteed to be <= 2*(3*m+5) = 6*m+10.
// Therefore, their product (= total number of nodes) is <= ((6*m+10)/2)^2 = (3*m+5)^2
void reduce(EndpointRepr* const er);



#endif
