#include <getopt.h>
#include <assert.h>

#include <mpi.h>

#include "graph.h"

label_t labels[MAX_VERTICES];
edge_t edges[MAX_EDGES];

int main(int argc, char *argv[]) {
	MPI_Init(NULL, NULL);

	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	const char *filename;

	bool undirected = false;

	for(int i; i < MAX_VERTICES; i++) {
		labels[i]=i;
	}
	int opt;
	while ((opt = getopt(argc, argv, "u")) != -1)
	{
		switch (opt) {
			case 'u':
				undirected = true;
				break;
			default:
				fprintf(stderr, "Usage: %s [-u] edgelist\n", argv[0]);
				exit(1);
		}
	}
	if (optind + 1 != argc) {
		fprintf(stderr, "Usage: %s [-u] edgelist\n", argv[0]);
		exit(1);
	}

	filename = argv[optind];


	FILE *file = fopen(filename, "r");

	if (file == NULL)
	{
		fprintf(stderr, "Failed to open graph file\n");
		exit(1);
	}

	size_t num_edges = 0;
	size_t num_vertices = 0;

	while (!feof(file)) {
		if (num_edges >= MAX_EDGES) {
			fprintf(stderr, "Graph has too many edges: more than num_edges %lu\n", num_edges);
			exit(1);
		}
		edge_t edge;
		fscanf(file, "%lu %lu\n", &edge.from, &edge.to);
		if (edge.from > num_vertices)
			num_vertices = edge.from;
		if (edge.to  > num_vertices) {
			num_vertices = edge.to;
		}
		edges[num_edges++] = edge;
		if (undirected && num_edges < MAX_EDGES) {
			edges[num_edges].from = edge.to;
			edges[num_edges++].to = edge.from;
		}
	}

	// +1 because vertex ID starts at 0
	num_vertices += 1;

	if (num_vertices >= MAX_VERTICES) {
		fprintf(stderr, "Too many vertices: %lu\n", num_vertices);
		exit(1);
	}

	printf("num_edges = %lu\n", num_edges);
	printf("num_vertices = %lu\n", num_vertices);

	assert(num_edges < MAX_EDGES);

	/* label_prop(); */

	size_t num_partitions = 0;
	for (size_t i = 0; i < num_vertices; i++) {
		//printf("%d ", labels[i]);
		if (labels[i] == i) {
			num_partitions++;
			printf("%lu\n", i);
		}
	}
	printf("num_partitions: %lu\n", num_partitions);

	MPI_Finalize();
}
