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

MPI_Datatype mpi_edge_t;

static uint64_t labels[MAX_VERTICES];
static edge_t edges[MAX_EDGES];
static size_t num_vertices;
static size_t num_edges;
static bool split_edges;
static int world_rank;
static int world_size;


static inline int vtor(int vertex_idx) {
    return (vertex_idx) * world_size / num_vertices;
}

static inline int rtov_upper(int rank) {
    return num_vertices * (world_rank + 1) / world_size;
}

static inline int rtov_lower(int rank) {
    return num_vertices * world_rank / world_size;
}


void main(int argc, char *argv[]) {
    split_edges = false;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <graphfile>\n", argv[0]);
        return;
    }

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    const int nitems = 2;
    int blocklengths[2] = {1, 1};
    MPI_Datatype types[2] = {MPI_UNSIGNED_LONG, MPI_UNSIGNED_LONG};
    MPI_Aint offsets[2];
    offsets[0] = 0;
    // hack, but it works
    offsets[1] = sizeof(((edge_t *) 0)->from);
    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_edge_t);
    MPI_Type_commit(&mpi_edge_t);

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

    printf("%i: number of partitions: %zu\n", world_rank, num_partitions);

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
            MPI_Request send_request;
            MPI_Isend(&e, 1, mpi_edge_t, rto, 0, MPI_COMM_WORLD, &send_request);
        } else {
            // edge touches two vertices in this rank
            labels[e.to] = labels[e.from];
        }
        edge_t edge_recv;
        int num_recved;
        int flag;
        MPI_Status stat;

        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &stat);

        while (flag) {
            MPI_Recv(&edge_recv, 1, mpi_edge_t, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
            printf("%i: %lu -> %lu\n", world_rank, edge_recv.from, edge_recv.to);
            labels[edge_recv.to] = edge_recv.from;
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &stat);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    edge_t edge_recv;
    int num_recved;
    int flag;
    MPI_Status stat;
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &stat);

    while (flag) {
        MPI_Recv(&edge_recv, 1, mpi_edge_t, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
        printf("%i: %lu -> %lu\n", world_rank, edge_recv.from, edge_recv.to);
        labels[edge_recv.to] = edge_recv.from;
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &stat);
    }

    MPI_Barrier(MPI_COMM_WORLD);
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
    for (i = rtov_lower(world_rank); i < rtov_upper(world_rank); i++) {
        if (labels[i] == i) {
            num_partitions++;
        }
    }
    return num_partitions;
}

void print_labels() {
    size_t i;
    for (i = rtov_lower(world_rank); i < rtov_upper(world_rank); i++) {
        printf("%i: labels[%zu] = %lu\n", world_rank, i, labels[i]);
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
