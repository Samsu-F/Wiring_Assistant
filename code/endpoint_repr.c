#include "endpoint_repr.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>



// Comparison function for qsort
// compare 2 int_fast32_t** by the value they are pointing to
static int compare_int_fast32_t_ptr(const void* a, const void* b)
{
    const int_fast32_t x = **(const int_fast32_t* const*)a;
    const int_fast32_t y = **(const int_fast32_t* const*)b;
    return (x > y) - (x < y);
}



// given a pointer to an array of long*, reduce and update all the long
// lower bound of -1 is assumend and does not have to be included
static void reduce_worker(int_fast32_t* arr[], size_t length)
{
    qsort(arr, length, sizeof(int_fast32_t*), compare_int_fast32_t_ptr);
    int_fast32_t prev_val = -1; // the previous value to compare the current value to
    int_fast32_t sum_shifts = 0; // the sum of all shifts done so far, i.e. this has to be substracted from
                                 // the rest of the values
    for(size_t i = 0; i < length; i++) {
        *(arr[i]) -= sum_shifts; // apply all previously found shifts
        int_fast32_t diff = *(arr[i]) - prev_val;
        if(diff >= 3) {
            int_fast32_t shift = diff - 2;
            // shift has to be subtracted from the rest of the array beginning at i
            *(arr[i]) -= shift;
            sum_shifts += shift;
        }
        prev_val = *(arr[i]);
    }
}



// Simplify the grid by removing identical neighboring columns/rows.
// Must not be called on a nullpointer.
// For number of existing wires m, after this function both the width and the height of the grid are
// guaranteed to be equal to or less than 4*m+5. Since there can only be at most 3 unique
// coordinates per cable, their sum is guaranteed to be <= 2*(3*m+5) = 6*m+10.
// Therefore, their product (= total number of nodes) is <= ((6*m+10)/2)^2 = (3*m+5)^2
void reduce(EndpointRepr* const er)
// create arrays for x- and y-coordinates, fill them with all the values and call reduce_worker
// on them to do the main work.
{
    assert(er != NULL && er->m > 0 && er->wires != NULL);
    size_t n = 2 * er->m + 3; // the number of coordinates per direction

    // use array of long* so the original long values can be changed when going through the array
    int_fast32_t* xs[n]; // worst case size for these two VLAs is 1608 bytes each,
    int_fast32_t* ys[n]; // assuming sizeof(int_fast32_t*) = 8. So unless ran on a very limited embedded system,
                         // the max stack size will definitely not be a problem.
    for(int i = 0; i < er->m; i++) {
        xs[2 * i] = &(er->wires[i].x1);
        ys[2 * i] = &(er->wires[i].y1);
        xs[2 * i + 1] = &(er->wires[i].x2);
        ys[2 * i + 1] = &(er->wires[i].y2);
    }
    // include upper bound but not lower bound (-1)
    xs[n - 3] = &(er->width);
    ys[n - 3] = &(er->height);
    xs[n - 2] = &(er->p1x);
    ys[n - 2] = &(er->p1y);
    xs[n - 1] = &(er->p2x);
    ys[n - 1] = &(er->p2y);

    reduce_worker(xs, n);
    reduce_worker(ys, n);
}
