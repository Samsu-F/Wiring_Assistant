#ifndef _COORDINATE_STRUCT_H
#define _COORDINATE_STRUCT_H



#include <stddef.h>



// Representation of a wire by the coordinates of its end points
// It must hold that x1 <= x2 and y1 <= y2
typedef struct Wire {
    long x1;
    long y1;
    long x2;
    long y2;
} Wire;



// Representation of a problem instance by all relevant coordinates, wires and values.
// This form of representaion is useful for parsing and reduction but it is not intended for
// running a pathfinding algorithm on it.
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



// Simplify the grid by removing identical neighboring columns/rows.
// Must not be called on a nullpointer.
// For number of existing wires m, after this function both the width and the height of the grid are
// guaranteed to be equal to or less than 4*m+5. Since there can only be at most 3 unique
// coordinates per cable, their sum is guaranteed to be <= 2*(3*m+5) = 6*m+10.
// Therefore, their product (= total number of nodes) is <= ((6*m+10)/2)^2 = (3*m+5)^2
void reduce(RawData* const rd);



#endif
