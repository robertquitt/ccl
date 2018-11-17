#include <pthread.h>
#include <assert.h>
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

void test_helpers() {
}

void *top_wrapper(void *threadarg) {
	struct thread_data *my_data;
	my_data = (struct thread_data *) threadarg;
	top(my_data->ctrl, my_data->edges, my_data->input, my_data->output,
			my_data->labels, my_data->world_rank, my_data->world_size,
			my_data->num_edges, my_data->num_vertices);
	pthread_exit(NULL);
}

void *cpu_sim(void *threadarg) {
	struct thread_data *my_data = (struct thread_data *) threadarg;

	ctrl_t *ctrl = my_data->ctrl;
	while (1) {
		// wait for fpga to have data ready to send
		while (!ctrl->output_valid) {
			;
		}

		if (ctrl->converged) {
			printf("cpu: done <- true\n");
			ctrl->done = 1;
			// tell fpga data has been sent
			printf("cpu: output_valid <- false\n");
			ctrl->output_valid = 0;
			break;
		}

		// tell fpga data has been sent
		printf("cpu: output_valid <- false\n");
		ctrl->output_valid = 0;
	}
	printf("cpu: done\n");
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

	const char *filename = "facebook_combined.txt";
	const bool undirected = true;

	FILE *file = fopen(filename, "r");

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

	printf("num_edges = %lu\n", num_edges);
	printf("num_vertices = %lu\n", num_vertices);

	assert(num_edges > MAX_VERTICES);

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
	printf("num_partitions: %lu\n", num_partitions);
}
