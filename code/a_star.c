#include "a_star.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <limits.h>



typedef struct PathCost {
    uint16_t intersections;
    uint16_t length;
} PathCost;

#define PQ_KEY_TYPE PathCost
#define PQ_VAL_TYPE Uint16Point
#include "pqueue.h"



// comparison function for the priority queue used in the path search
static bool cheaper_path(const PathCost a, const PathCost b)
{
    uint32_t metric_a = a.length | ((uint32_t)a.intersections << (sizeof(a.length) * CHAR_BIT));
    uint32_t metric_b = b.length | ((uint32_t)b.intersections << (sizeof(b.length) * CHAR_BIT));
    // guaranteed to be true as long as nothing is changed, but this might catch a mistake if the types are changed
    static_assert(sizeof(metric_a) >= sizeof(a.intersections) + sizeof(a.length));
    return metric_a < metric_b;
}



static void** new_matrix(const size_t width, const size_t height, const uint8_t init_byte_value, size_t sizeoftype)
{
    static_assert(sizeof(void*) == sizeof(void**)); // you would have to use a very strange system for this to fail, but the C standard does not guarantee it
    void** matrix = malloc(width * sizeof(void*));
    if(!matrix) {
        return NULL;
    }
    matrix[0] = malloc(width * height * sizeoftype);
    if(!matrix[0]) {
        free(matrix);
        return NULL;
    }
    memset(matrix[0], init_byte_value, width * height * sizeoftype);
    for(size_t x = 1; x < width; x++) {
        matrix[x] = (void*)((char*)(matrix[0]) + (x * height * sizeoftype));
    }
    return matrix;
}

static void free_matrix(void** matrix)
{
    free(matrix[0]);
    free(matrix);
}



// wrapper function
// Allocate and initialize a scores table for the A* algorithm.
// May return NULL if allocation failed.
// Caller is responsible for freeing the table again with free_scores_table [see below].
static PathCost** new_scores_table(const size_t width, const size_t height, const uint8_t init_byte_value)
{
    static_assert((sizeof(PathCost**) == sizeof(void*)) && (sizeof(PathCost*) == sizeof(void*)));
    return (PathCost**)new_matrix(width, height, init_byte_value, sizeof(PathCost));
}
// wrapper function, free a scores table that was allocated by new_scores_table and all of its internal allocations
static void free_scores_table(PathCost** scores)
{
    free_matrix((void**)scores);
}


// wrapper function
// Allocate and initialize a predecessor table that can be used in the A* algorithm
// if the cheapest path taken should be reconstructed.
// May return NULL if allocation failed.
// Caller is responsible for freeing the table again with free_predecessor_table [see below].
static Uint16Point** new_predecessor_table(const size_t width, const size_t height, const uint8_t init_byte_value)
{
    static_assert((sizeof(Uint16Point**) == sizeof(void*)) && (sizeof(Uint16Point*) == sizeof(void*)));
    return (Uint16Point**)new_matrix(width, height, init_byte_value, sizeof(Uint16Point));
}
// wrapper function
// free a predecessor_table table that was allocated by new_predecessor_table and all of its internal allocations
static void free_predecessor_table(Uint16Point** pred_tbl)
{
    free_matrix((void**)pred_tbl);
}



// wrapper function
bool** new_path_map(const size_t width, const size_t height)
{
    static_assert((sizeof(bool**) == sizeof(void*)) && (sizeof(bool*) == sizeof(void*)));
    return (bool**)new_matrix(width, height, (const uint8_t) false, sizeof(bool));
}
// wrapper function
void free_path_map(bool** path_map)
{
    free_matrix((void**)path_map);
}



// Calculate the minimal cost possible for a path between p1 and p2, where the cost of a path is
// defined as the sum of the node costs of all the nodes in the path, including start and end.
static int16_t a_star(const Graph* const g, HeuristicFunc h, Uint16Point** pred_tbl)
{
    const Uint16Point p1 = g->p1;
    const Uint16Point p2 = g->p2;

    PQueue* openset = pq_new(cheaper_path); // path costs are keys, node ids are values
    assert(openset != NULL);
    PathCost path_cost_p1 = {.intersections = g->node_cost[p1.x][p1.y], .length = 0};
    pq_insert(openset, (KeyValPair) {.key = path_cost_p1, .val = p1});

    PathCost** g_scores = new_scores_table(g->width, g->height, 0xFF);
    if(!g_scores) {
        fprintf(stderr, "Allocation for g_scores table failed.\n");
        exit(EXIT_FAILURE);
    }

    g_scores[p1.x][p1.y] = path_cost_p1;

    while(!pq_is_empty(openset)) {
        const KeyValPair current = pq_pop(openset);
        const Uint16Point cur_point = current.val;
        const PathCost cur_g_score = g_scores[cur_point.x][cur_point.y];
        if(cur_point.x == p2.x && cur_point.y == p2.y) { // if current point is goal
            pq_free(openset);
            free_scores_table(g_scores);
            return (int16_t)current.key.intersections;
        }

        Uint16Point neighbors[4];
        int neigh_count = 0;
        const uint8_t cur_neighbors_bitmap = g->neighbors[cur_point.x][cur_point.y];
        // for each direction, check if there is an edge
        if(cur_neighbors_bitmap & NEIGH_X_NEG) {
            neighbors[neigh_count++] = (Uint16Point) {cur_point.x - 1, cur_point.y};
        }
        if(cur_neighbors_bitmap & NEIGH_X_POS) {
            neighbors[neigh_count++] = (Uint16Point) {cur_point.x + 1, cur_point.y};
        }
        if(cur_neighbors_bitmap & NEIGH_Y_NEG) {
            neighbors[neigh_count++] = (Uint16Point) {cur_point.x, cur_point.y - 1};
        }
        if(cur_neighbors_bitmap & NEIGH_Y_POS) {
            neighbors[neigh_count++] = (Uint16Point) {cur_point.x, cur_point.y + 1};
        }
        // for each neighbor of current
        for(int i = 0; i < neigh_count; i++) {
            const Uint16Point neighbor = neighbors[i];
            const PathCost tent_g_score = {cur_g_score.intersections +
                                               g->node_cost[neighbor.x][neighbor.y],
                                           cur_g_score.length + 1};
            if(cheaper_path(tent_g_score, g_scores[neighbor.x][neighbor.y])) {
                g_scores[neighbor.x][neighbor.y] = tent_g_score;
                PathCost neigh_f_score = {tent_g_score.intersections, tent_g_score.length + h(neighbor, p2)};
                pq_insert(openset, (KeyValPair) {neigh_f_score, neighbor});
                if(pred_tbl) {
                    pred_tbl[neighbor.x][neighbor.y] = (Uint16Point) {cur_point.x, cur_point.y};
                }
            }
        }
    }
    // this point is only reached if there is no connection from p1 to p2
    pq_free(openset);
    free_scores_table(g_scores);
    return -1;
}

// wrapper for public interface for situations where only the cost
// of the cheapest path is needed.
int16_t a_star_cost(const Graph* const g, HeuristicFunc h)
{
    return a_star(g, h, NULL);
}



// wrapper for public interface for situations where the cost
// of the cheapest path as well as the path map needed.
int16_t a_star_path_map(const Graph* const g, HeuristicFunc h, bool** path_map)
{
    // just some assertions and init
    if(!path_map) {
        return a_star(g, h, NULL); // don't crash if caller violates contract to provide pointer
    }
    for(uint16_t x = 0; x < g->width; x++) {
        assert(path_map[x]);
    }
    Uint16Point** pred_tbl = new_predecessor_table(g->width, g->height, 0xFF);
    if(!pred_tbl) {
        fprintf(stderr, "Allocation for predecessor table failed.\n");
        exit(EXIT_FAILURE);
    }

    // run A*
    int16_t cost_result = a_star(g, h, pred_tbl);

    // reconstruct the cheapest path, starting from the goal (p2) and going back
    if(cost_result >= 0) { // if there is a path
        uint16_t x = g->p2.x;
        uint16_t y = g->p2.y;
        while(!(x == g->p1.x && y == g->p1.y)) { // while we haven't reached the start (p1) yet
            path_map[x][y] = true;
            Uint16Point predecessor = pred_tbl[x][y];
            x = predecessor.x;
            y = predecessor.y;
        }
        path_map[x][y] = true; // mark the start as well
    }

    free_predecessor_table(pred_tbl);
    return cost_result;
}
