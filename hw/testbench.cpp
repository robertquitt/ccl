#include <pthread.h>
#include "graph.h"

struct thread_data {
	ctrl_t* ctrl;
	edge_t* edges;
	info_t* input;
	info_t* output;
	ap_uint<64>* labels;
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

	my_data->ctrl->input_valid = 1;
	my_data->ctrl->output_valid = 0;
	my_data->ctrl->done = 1;

	pthread_exit(NULL);
}

edge_t edges[MAX_EDGES];
info_t input[MAX_EDGES];
info_t output[MAX_EDGES];
ap_uint<64> labels[MAX_VERTICES];

int main() {
	pthread_t threads[4];
	struct thread_data my_data[4];
	int rc;

	ctrl_t ctrl[2];

	my_data[0].ctrl = &ctrl[0];
	my_data[0].edges = edges;
	my_data[0].input = input;
	my_data[0].output = output;
	my_data[0].labels = labels;
	my_data[0].world_rank = 0;
	my_data[0].world_size = 2;
	my_data[0].num_edges = 10;
	my_data[0].num_vertices = 10;

	ctrl[0].input_size = 0;
	ctrl[0].output_size = 0;
	ctrl[0].converged = 0;
	ctrl[0].done = 0;
	ctrl[0].input_valid = 0;
	ctrl[0].output_valid = 0;

//	FILE *file = fopen("n100p0.1.txt", "r");
	FILE *file = fopen("facebook_combined.txt", "r");

	size_t num_edges = 0;

	while (!feof(file) && num_edges < MAX_EDGES) {
		edge_t edge;
		fscanf(file, "%llu %llu\n", &edge.from, &edge.to);
		edges[num_edges++] = edge;
	}

	printf("Number of edges: %lu\n", num_edges);

	printf("Edge[0]: %llu %llu\n", edges[0].from, edges[0].to);

	rc = pthread_create(&threads[0], NULL, top_wrapper, (void *) &my_data[0]);
	assert(!rc);
	rc = pthread_create(&threads[1], NULL, cpu_sim, (void *) &my_data[0]);
	assert(!rc);

#ifdef CCL_TWO_NODES
	my_data[1].ctrl = &ctrl[1];
	my_data[1].edges = edges;
	my_data[1].input = input;
	my_data[1].output = output;
	my_data[1].labels = labels;
	my_data[1].world_rank = 1;
	my_data[1].world_size = 2;
	my_data[1].num_edges = 10;
	my_data[1].num_vertices = 10;

	ctrl[1].input_size = 0;
	ctrl[1].output_size = 0;
	ctrl[1].converged = 0;
	ctrl[1].done = 0;
	ctrl[1].input_valid = 0;
	ctrl[1].output_valid = 0;
	rc = pthread_create(&threads[2], NULL, top_wrapper, (void *) &my_data[1]);
	assert(!rc);
	rc = pthread_create(&threads[3], NULL, cpu_sim, (void *) &my_data[1]);
	assert(!rc);
#endif

	for (size_t i = 0; i < 4; i++) {
		pthread_join(threads[i], NULL);
	}
}
