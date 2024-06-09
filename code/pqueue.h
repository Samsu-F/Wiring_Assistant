#ifndef _PQUEUE_H
#define _PQUEUE_H


/*
 * Array based heap implementation of a priority queue
 */



#include <stdbool.h>
#include <stdint.h>


#ifndef PQ_KEY_TYPE
#define PQ_KEY_TYPE void*
#endif

#ifndef PQ_VAL_TYPE
#define PQ_VAL_TYPE void*
#endif


#define PQ_INIT_SIZE        64 // start with enough space for x key value pairs
#define PQ_REALLOC_FACTOR   2  // multiply the size by x if more space is needed
#define PQ_REALLOC_DIVISOR  2  // divide the size by x if space is no longer needed
#define PQ_DEALLOCATE_LIMIT 4  // dealloc space if less than 1/x of the allocated space is needed



typedef PQ_KEY_TYPE pq_keytype;
typedef PQ_VAL_TYPE pq_valtype;


typedef struct KeyValPair {
    pq_keytype key;
    pq_valtype val;
} KeyValPair;

typedef bool (*PQKeyCompareFunc)(const pq_keytype, const pq_keytype);

typedef struct PQueue PQueue;


// compare is a function to compare keys
// compare(key1, key2) == true iff key1 has greater priority than key2, i.e. it will be popped sooner
PQueue* pq_new(PQKeyCompareFunc compare);

void pq_free(PQueue* q);

bool pq_is_empty(const PQueue* q);

void pq_insert(PQueue* q, const KeyValPair new);

// must not be called on an empty PQueue
KeyValPair pq_peek(const PQueue* q);

// must not be called on an empty PQueue
KeyValPair pq_pop(PQueue* q);



#endif
