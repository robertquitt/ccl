#include<stdint.h>
#include<stdio.h>
#include "mpi.h"

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

void label_graph();
void load_graph();

void main(int argc, char *argv[]) {
	MPI_Init(&argc, &argv);

	load_graph();

	size_t i;
	for (i = 0; i < num_vertices; i++) {
		labels[i] = i;
	}

	label_graph();

	size_t num_partitions = 0;

	for (i = 0; i < num_vertices; i++) {
		printf("labels[%zu] = %lu\n", i, labels[i]);
		if (labels[i] == i) {
			num_partitions++;
		}
	}

	printf("Number of partitions: %zu\n", num_partitions);


	MPI_Finalize();
}

void label_graph() {
	size_t i;
	for (i = 0; i < num_edges; i++) {
		edge_t e = edges[i];
		labels[e.to] = labels[e.from];
	}
}

/*
 * Loads the graph into memory from stdin. Expect number of vertices then list
 * of edges.
 */
void load_graph() {
	scanf("%u", (unsigned int *) &num_vertices);
	size_t i;
	edge_t e;
	for (i = 0; i < MAX_EDGES; i++) {
		scanf("%lu %lu", &e.from, &e.to);
		if (feof(stdin)) {
			break;
		}
		edges[i] = e;
	}
	num_edges = i;
}

/* vi: sw=4 ts=4 et */
