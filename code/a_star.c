#include "a_star.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "coordinate_struct.h"


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
    return a.intersections < b.intersections || (a.intersections == b.intersections && a.length < b.length);
}



static PathCost** new_scores_table(const size_t width, const size_t height, const uint8_t init_byte_value)
{
    PathCost** scores = malloc(width * sizeof(PathCost*));
    if(!scores) {
        return NULL;
    }
    scores[0] = malloc(width * height * sizeof(PathCost));
    if(!scores[0]) {
        free(scores);
        return NULL;
    }
    scores[0][width * height - 1] = (PathCost) {42, 42};
    memset(scores[0], init_byte_value, width * height * sizeof(PathCost));
    for(size_t x = 1; x < width; x++) {
        scores[x] = scores[0] + (x * height);
    }
    return scores;
}

static void free_scores_table(PathCost** scores)
{
    free(scores[0]);
    free(scores);
}



// calculate the minimal cost possible for a path between p1 and p2, where the cost of a path is
// defined as the sum of the node costs of all the nodes in the path, including start and end
int a_star_cost(const Graph* const g, HeuristicFunc h)
// A* search algorithm without recontructing the path or keeping track of the predecessor node
{
    const Uint16Point p1 = g->p1;
    const Uint16Point p2 = g->p2;

    PQueue* openset = pq_new(cheaper_path); // path costs are keys, node ids are values
    assert(openset != NULL);
    PathCost path_cost_p1 = {.intersections = g->node_cost[p1.x][p1.y], .length = 0};
    pq_insert(openset, (KeyValPair) {.key = path_cost_p1, .val = p1});

    PathCost** g_scores = new_scores_table(g->width, g->height, 0xFF);
    assert(g_scores);

    g_scores[p1.x][p1.y] = path_cost_p1;

    while(!pq_is_empty(openset)) {
        const KeyValPair current = pq_pop(openset);
        const Uint16Point cur_point = current.val;
        const PathCost cur_g_score = g_scores[cur_point.x][cur_point.y];
        if(cur_point.x == p2.x && cur_point.y == p2.y) { // if current point is goal
            pq_free(openset);
            free_scores_table(g_scores);
            return current.key.intersections;
        }

        Uint16Point neighbors[4];
        int neigh_count = 0;
        const uint8_t cur_neighbors_bitmap = g->neighbors[cur_point.x][cur_point.y];
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
        // for each neighbor off current
        for(int i = 0; i < neigh_count; i++) {
            const Uint16Point neighbor = neighbors[i];
            const PathCost tent_g_score = {cur_g_score.intersections +
                                               g->node_cost[neighbor.x][neighbor.y],
                                           cur_g_score.length + 1};
            if(cheaper_path(tent_g_score, g_scores[neighbor.x][neighbor.y])) {
                g_scores[neighbor.x][neighbor.y] = tent_g_score;
                PathCost neigh_f_score = {tent_g_score.intersections, tent_g_score.length + h(neighbor, p2)};
                pq_insert(openset, (KeyValPair) {neigh_f_score, neighbor});
            }
        }
    }
    // this point is only reached if there is no connection from p1 to p2
    pq_free(openset);
    free_scores_table(g_scores);
    return -1;
}
