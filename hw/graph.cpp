#include "graph.h"

//1. All edges point to me
//2. 4 parts of vertices
//	a. myself-myself
//	b. send the data from the proc2/proc3/proc4 to me
//	c. process the lables

static inline int vtor(int vertex_idx, int world_size, int num_vertices) {
	return (vertex_idx) * world_size / num_vertices;
}

static inline int rtov_upper(int rank, int world_size, int num_vertices) {
	return num_vertices * (rank + 1) / world_size;
}

static inline int rtov_lower(int rank, int world_size, int num_vertices) {
	return num_vertices * rank / world_size;
}

// Convert directed graph -> undirected graph
// strongly connected components -> weakly connected components (Parconnect)
// adding opposite direction to the graph data
// ctrl[0] -> accel output valid signal
// ctrl[1] -> accel input valid signal
// output_size -> largest buffer size, but we are not using it
void top(ctrl_t* ctrl, edge_t* edges, info_t* input, info_t* output, label_t*
		labels, int world_rank, int world_size, int num_edges,
		int num_vertices) {
#pragma HLS INTERFACE s_axilite port=return bundle=control
#pragma HLS INTERFACE s_axilite port=edges bundle=control
#pragma HLS INTERFACE m_axi port=ctrl offset=slave depth=128 bundle=gmem0
#pragma HLS INTERFACE s_axilite port=ctrl bundle=control
#pragma HLS INTERFACE m_axi port=edges offset=slave depth=128 bundle=gmem0
#pragma HLS INTERFACE s_axilite port=input bundle=control
#pragma HLS INTERFACE m_axi port=input offset=slave depth=128 bundle=gmem1
#pragma HLS INTERFACE s_axilite port=output bundle=control
#pragma HLS INTERFACE m_axi port=output offset=slave depth=128 bundle=gmem2
#pragma HLS INTERFACE s_axilite port=labels bundle=control
#pragma HLS INTERFACE m_axi port=labels offset=slave depth=128 bundle=gmem3

#pragma HLS INTERFACE s_axilite port=world_rank bundle=control
#pragma HLS INTERFACE s_axilite port=world_size bundle=control
#pragma HLS INTERFACE s_axilite port=num_edges bundle=control
#pragma HLS INTERFACE s_axilite port=num_vertices bundle=control

	bit_t converged = 1;
	label_t local_labels[MAX_VERTICES];
	//#pragma HLS ARRAY_PARTITION variable=local_labels complete dim=0

	int offset = rtov_lower(world_rank, world_size, num_vertices);

	for (int i = offset;
			i < rtov_upper(world_rank, world_size, num_vertices); i++) {
		//#pragma HLS UNROLL factor=16
		local_labels[i - offset] = i;
	}
	printf("offset = %i\n", offset);
	printf("rtov_upper = %i\n", rtov_upper(world_rank, world_size, num_vertices));

	while (1) {

		int count = 0;
		for (int i = 0; i < num_edges; i++) {
			//#pragma HLS UNROLL factor=16
			edge_t e = edges[i];

			int rto = vtor(e.to, world_size, num_vertices);
			/* int rfrom = vtor(e.from, world_size, num_vertices); */
			// assert(rfrom == world_rank);
			// Make sure the rfrom is always on the current processor when we partition the workload
			if (rto != world_rank) {
				info_t info;
				info.to = e.to;
				info.label = local_labels[e.from - offset];
				// destination vertex + label of rfrom if we have more than 2 processors, we need to have more buffers
				output[count++] = info;
			} else {
				// edge touches two vertices in this rank
				if (local_labels[e.from - offset]
						< local_labels[e.to - offset]) {
					local_labels[e.to - offset] = local_labels[e.from - offset];
					converged = 0;
				}
			}
		}

		ctrl->output_size = count;
		count = 0;

		ctrl->converged = converged;
		printf("top: converged <- %i\n", converged);

		// tell proc that we are ready to send out data
		printf("top: output_valid <- true\n");
		ctrl->output_valid = 1;

		// poll to see if there is any incoming data

		/* printf("top waiting for valid input\n"); */
		/* while (!(ctrl->input_valid)) { */
		/* 	; */
		/* } */

		// process incoming data
		/* size_t input_size = ctrl->input_size; */
		/* for (size_t i = 0; i < input_size; i++) { */
		/* 	info_t in = input[i]; */
		/* 	if (local_labels[in.to - offset] != in.label) { */
		/* 		local_labels[in.to - offset] = in.label; */
		/* 		//    		label_updates[in.to - offset] = 1; */
		/* 		converged = 0; */
		/* 	} */
		/* } */

		/* printf("top processed incoming data\n"); */

		/* ctrl->input_valid = 0; */

		// wait till send is done on the proc end
		while (ctrl->output_valid) {
			;
		}
		converged = 1;

		if (ctrl->done) {
			for (int i = offset;
					i < rtov_upper(world_rank, world_size, num_vertices); i++) {
#pragma HLS UNROLL factor=16
				labels[i] = local_labels[i - offset];
			}
			return;
		}
	}
}
