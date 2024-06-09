#ifndef _COORDINATE_STRUCT_H
#define _COORDINATE_STRUCT_H



#include <stddef.h>



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



void reduce(RawData* const rd);



#endif
