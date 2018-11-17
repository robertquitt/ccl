#include <pthread.h>
#include "graph.h"

struct thread_data {
	ctrl_t* ctrl;
	edge_t* edges;
	info_t* input;
	info_t* output;
	label_t* labels;
	int world_rank;
	int world_size;
	int num_edges;
	int num_vertices;
};

void *top_wrapper(void *threadarg) {
	struct thread_data *my_data;
	my_data = (struct thread_data *) threadarg;
	top(my_data->ctrl, my_data->edges, my_data->input, my_data->output,
			my_data->labels, my_data->world_rank, my_data->world_size,
			my_data->num_edges, my_data->num_vertices);
	pthread_exit(NULL);
}

void *cpu_sim(void *threadarg) {
	struct thread_data *my_data;

	ctrl_t *ctrl = my_data->ctrl;
	while (1) {
		printf("CPU waiting for output from top\n");
		while (!ctrl->output_valid) {
			;
		}
		printf("CPU output size: %zu\n", ctrl->output_size);
		ctrl->input_valid = 1;
		while (ctrl->input_valid) {
			;
		}
		ctrl->output_valid = 0;
		ctrl->done = 1;
	}
	printf("CPU done\n");
	pthread_exit(NULL);
}

edge_t edges[MAX_EDGES];
size_t num_edges;
info_t input[MAX_EDGES];
info_t output[MAX_EDGES];
label_t labels[MAX_VERTICES];

/**
 * Simulates CPU-Accelerator coprocessing system
 * Each CPU/Accelerator is a pthread
 * threads[0]: cpu 0
 * threads[1]: accel 0
 */
#define NUM_NODES 1

int main() {
	pthread_t threads[2 * NUM_NODES];
	struct thread_data my_data[NUM_NODES];
	ctrl_t ctrl[NUM_NODES];

	FILE *file = fopen("n100p0.1.txt", "r");
//	FILE *file = fopen("facebook_combined.txt", "r");

	if (file == NULL)
	{
		fprintf(stderr, "Failed to open graph file\n");
		exit(1);
	}

	num_edges = 0;
	size_t num_vertices = 0;

	while (!feof(file) && num_edges < MAX_EDGES) {
		edge_t edge;
		fscanf(file, "%lu %lu\n", &edge.from, &edge.to);
		if (edge.from > num_vertices)
			num_vertices = edge.from;
		if (edge.to > num_vertices) {
			num_vertices = edge.to;
		}
		edges[num_edges++] = edge;
	}

	my_data[0].ctrl = &ctrl[0];
	my_data[0].edges = edges;
	my_data[0].input = input;
	my_data[0].output = output;
	my_data[0].labels = labels;
	my_data[0].world_rank = 0;
	my_data[0].world_size = 1;
	my_data[0].num_edges = num_edges;
	my_data[0].num_vertices = num_vertices;

	ctrl[0].input_size = 0;
	ctrl[0].output_size = 0;
	ctrl[0].converged = 0;
	ctrl[0].done = 0;
	ctrl[0].input_valid = 0;
	ctrl[0].output_valid = 0;

	pthread_create(&threads[0], NULL, cpu_sim, (void *) &my_data[0]);
	pthread_create(&threads[1], NULL, top_wrapper, (void *) &my_data[0]);

	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);

	size_t num_partitions = 0;
	for (size_t i = 0; i < num_vertices; i++) {
		if (labels[i] == i) {
			num_partitions++;
		}
	}
	printf("num_partitions: %zu\n", num_partitions);
}
