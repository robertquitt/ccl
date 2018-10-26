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
	int num_vertice;
};

void *top_wrapper(void *threadarg) {
   struct thread_data *my_data;
   my_data = (struct thread_data *) threadarg;
   printf("Pthread run\n");
   top(my_data->ctrl, my_data->edges, my_data->input, my_data->output, my_data->labels,
		   my_data->world_rank, my_data->world_size, my_data->num_edges, my_data->num_vertice);
   pthread_exit(NULL);
}

#define NUM_THREADS 1
int main () {
   pthread_t threads[NUM_THREADS];
   struct thread_data my_data[NUM_THREADS];
   int rc;

   ctrl_t ctrl[1];
   edge_t edges[MAX_EDGES];
   info_t input[MAX_EDGES];
   info_t output[MAX_EDGES];
   ap_uint<64> labels[MAX_VERTICES];

   my_data->ctrl = ctrl;
   my_data->edges = edges;
   my_data->input = input;
   my_data->output = output;
   my_data->labels = labels;
   my_data->world_rank = 0;
   my_data->world_size = 2;
   my_data->num_edges = 0;
   my_data->num_vertice = 0;

   for(size_t i = 0; i < NUM_THREADS; i++ ) {
	      rc = pthread_create(&threads[i], NULL, top_wrapper, (void *)&my_data[i]);
	      if (rc) {
	         printf("Error:unable to create thread %d\n", rc);
	         exit(-1);
	      }
   }
   my_data->ctrl->input_valid = 1;
   my_data->ctrl->output_valid = 0;
   my_data->ctrl->done = 1;

   pthread_join(threads[0], NULL);
}
