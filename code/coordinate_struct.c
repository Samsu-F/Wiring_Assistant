#include "coordinate_struct.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>



// Comparison function for qsort
// compare 2 long** by the value they are pointing to
static int compare_long_ptr(const void* a, const void* b)
{
    const long x = **(const long* const*)a;
    const long y = **(const long* const*)b;
    return (x > y) - (x < y);
}



// given a pointer to an array of long*, reduce and update all the long
// lower bound of -1 is assumend and does not have to be included
static void reduce_worker(long* arr[], size_t length)
{
    /// TODO: make it run in O(m) instead of O(m^2), but try to keep it easy to understand
    qsort(arr, length, sizeof(long*), compare_long_ptr);
    long prev = -1;
    for(size_t i = 0; i < length; i++) {
        long diff = *(arr[i]) - prev;
        if(diff >= 3) {
            // subtract shift from the rest of the array beginning at i
            long shift = diff - 2;
            for(size_t j = i; j < length; j++) {
                *(arr[j]) -= shift;
            }
        }
        prev = *(arr[i]);
    }
}



// Simplify the grid by removing identical neighboring columns/rows.
// Must not be called on a nullpointer.
// For number of existing wires m, after this function both the width and the height of the grid are
// guaranteed to be equal to or less than 4*m+5. Since there can only be at most 3 unique
// coordinates per cable, their sum is guaranteed to be <= 2*(3*m+5) = 6*m+10.
// Therefore, their product (= total number of nodes) is <= ((6*m+10)/2)^2 = (3*m+5)^2
void reduce(RawData* const rd)
// create arrays for x- and y-coordinates, fill them with all the values and call reduce_worker
// on them to do the main work.
{
    assert(rd != NULL && rd->m > 0 && rd->wires != NULL);
    size_t n = 2 * rd->m + 3; // the number of coordinates per direction

    // use array of long* so the original long values can be changed when going through the array
    long* xs[n]; // worst case size for these two VLAs is 1608 bytes each,
    long* ys[n]; // assuming sizeof(long*) = 8. So unless ran on a very limited embedded system,
                 // the max stack size will definitely not be a problem.
    for(int i = 0; i < rd->m; i++) {
        xs[2 * i] = &(rd->wires[i].x1);
        ys[2 * i] = &(rd->wires[i].y1);
        xs[2 * i + 1] = &(rd->wires[i].x2);
        ys[2 * i + 1] = &(rd->wires[i].y2);
    }
    // include upper bound but not lower bound (-1)
    xs[n - 3] = &(rd->width);
    ys[n - 3] = &(rd->height);
    xs[n - 2] = &(rd->p1x);
    ys[n - 2] = &(rd->p1y);
    xs[n - 1] = &(rd->p2x);
    ys[n - 1] = &(rd->p2y);

    reduce_worker(xs, n);
    reduce_worker(ys, n);
}
