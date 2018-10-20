#include<stdint.h>
#include<stdio.h>
#include<stdbool.h>
#include "mpi.h"
#include "ccl.h"

#define MAX_EDGES 1024
#define MAX_VERTICES 1024

typedef struct edge {
    uint64_t from;
    uint64_t to;
} edge_t;

static uint64_t labels[MAX_VERTICES];
static edge_t edges[MAX_EDGES];
static size_t num_vertices;
static size_t num_edges;
static bool split_edges;
static int world_rank;
static int world_size;

static inline int vtor(int vertex_idx) {
    return (vertex_idx + 1) * world_size / num_vertices;
}

static inline int rtov_upper(int rank) {
    return num_vertices * (world_rank + 1) / world_size;
}

static inline int rtov_lower(int rank) {
    return num_vertices * world_rank / world_size;
}


void main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    split_edges = false;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <graphfile>\n", argv[0]);
        return;
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    load_graph(argv[1]);

    if (world_size == 1) {
        run_labelprop();
    } else if (world_size == 2) {
        run_async_labelprop();
    } else {
        fprintf(stderr, "Run with 1 or 2 processors\n");
        return;
    }

    size_t num_partitions = count_partitions();

    print_labels();

    printf("Number of partitions: %zu\n", num_partitions);

    MPI_Finalize();
}

void run_async_labelprop() {
    size_t i;
    for (i = rtov_lower(world_rank); i < rtov_upper(world_rank); i++) {
        labels[i] = i;
    }
    for (i = 0; i < num_edges; i++) {
        edge_t e = edges[i];

        int rto = vtor(e.to);
        int rfrom = vtor(e.from);

        if (rfrom != world_rank) {
            // from vertex is on other processor, skip
            continue;
        } else if (rto != world_rank) {
            // write to other processor
            printf("%lu %lu\n", e.to, e.from);
        } else {
            labels[e.to] = labels[e.from];
        }
    }
}

void run_labelprop() {
    size_t i;
    for (i = 0; i < num_vertices; i++) {
        labels[i] = i;
    }
    for (i = 0; i < num_edges; i++) {
        edge_t e = edges[i];
        labels[e.to] = labels[e.from];
    }
}

size_t count_partitions() {
    size_t num_partitions = 0;
    size_t i;
    for (i = 0; i < num_vertices; i++) {
        if (labels[i] == i) {
            num_partitions++;
        }
    }
    return num_partitions;
}

void print_labels() {
    size_t i;
    for (i = 0; i < num_vertices; i++) {
        printf("labels[%zu] = %lu\n", i, labels[i]);
    }
}

/*
 * Loads the graph into memory from stdin. Expect number of vertices then list
 * of edges.
 */
void load_graph(char *filename) {
    FILE *graphfile = fopen(filename, "r");
    fscanf(graphfile, "%u", (unsigned int *) &num_vertices);
    size_t i;
    edge_t e;
    num_edges = 0;
    for (i = 0; i < MAX_EDGES; i++) {
        fscanf(graphfile, "%lu %lu", &e.from, &e.to);
        if (feof(graphfile)) {
            break;
        }

        if (!split_edges || i % world_size == world_rank) {
            edges[num_edges++] = e;
        }
    }
}

/* vi: sw=4 ts=4 et */
