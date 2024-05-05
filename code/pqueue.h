#ifndef MIN_PRIORITY_QUEUE
#define MIN_PRIORITY_QUEUE
/*
 * Min-heap implementation of a priority queue
 */



#include <stdbool.h>
#include <stdint.h>



#define PQ_INIT_SIZE        64 // start with enough space for x key value pairs
#define PQ_REALLOC_FACTOR   2  // multiply the size by x if more space is needed
#define PQ_REALLOC_DIVISOR  2  // divide the size by x if space is no longer needed
#define PQ_DEALLOCATE_LIMIT 4  // dealloc space if less than 1/x of the allocated space is needed



typedef uint16_t pq_keytype;
typedef uint16_t pq_valtype;

typedef struct KeyValPair {
    pq_keytype key;
    pq_valtype val;
} KeyValPair;

typedef struct PQueue PQueue;



PQueue* pq_allocate(void);

void pq_free(PQueue* q);

bool pq_is_empty(const PQueue* q);

void pq_insert(PQueue* q, const KeyValPair new);

// must not be called on an empty PQueue
KeyValPair pq_peek_min(const PQueue* q);

// must not be called on an empty PQueue
KeyValPair pq_pop_min(PQueue* q);



#endif
