#include "graph.h"

//1. All edges point to me
//2. 4 parts of vertices
//	a. myself-myself
//	b. send the data from the proc2/proc3/proc4 to me
//	c. process the lables

int vtor(int vertex_idx, int world_size, int num_vertices) {
	int trunk_size =  (num_vertices + world_size - 1) / world_size;
	return vertex_idx / trunk_size;
}

int rtov_upper(int rank, int world_size, int num_vertices) {
	int trunk_size =  (num_vertices + world_size - 1) / world_size;
	int upper = (rank+1) * trunk_size < num_vertices ? (rank+1) * trunk_size : num_vertices;
	return upper;
}

int rtov_lower(int rank, int world_size, int num_vertices) {
	int trunk_size =  (num_vertices + world_size - 1) / world_size;
	int lower = rank * (trunk_size) < num_vertices ? rank * (trunk_size): num_vertices;
	return lower;
}

/**
 * ctrl: control signal, currently in shared memory
 * edges: pointer to array of edges
 * input: pointer buffer to label updates to send from CPU->FPGA
 * output: pointers to buffer of label updates to send from FPGA->CPU
 * labels: pointer to array of labels (written to at end)
 * world_rank: effectively, this node's ID
 * world_size: number of nodes
 * num_edges: number of edges
 * num_vertices: number of TOTAL vertices, shared between all nodes.
 */
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
	label_prop(input, output, world_rank, world_size, num_vertices, ctrl, edges, num_edges, labels);
}

void label_prop(info_t *input, info_t *output, int world_rank, int world_size,
		int num_vertices, ctrl_t *ctrl, edge_t *edges, int num_edges, label_t *labels) {
	label_t local_labels[MAX_VERTICES];
	label_t label_buffer[MAX_VERTICES];
	char label_updates[MAX_VERTICES];
#pragma HLS ARRAY_PARTITION variable=local_labels complete dim=0

	int offset = rtov_lower(world_rank, world_size, num_vertices);

	for (int i = offset; i < rtov_upper(world_rank, world_size, num_vertices); i++) {
#pragma HLS UNROLL factor=16
		local_labels[i - offset] = i;
		label_updates[i - offset] = 1;
	}

	bit_t converged = 1;

	while (1) {
		int count = 0;
		for (int i = 0; i < num_edges; i++) {
#pragma HLS UNROLL factor=16
			edge_t e = edges[i];

			int rto = vtor(e.to, world_size, num_vertices);
			int rfrom = vtor(e.from, world_size, num_vertices);

			// TODO: partition the edges so that the "from" rank is
			// the same for a single node

			if (rto != world_rank && rfrom == world_rank
					&& label_updates[e.from - offset]) {
				info_t info;
				info.to = e.to;
				info.label = local_labels[e.from - offset];
				output[count++] = info;
				label_updates[e.from - offset] = 0;
				label_buffer[e.from] = local_labels[e.from - offset];
				printf("[%i]: send from %lu(%lu) to %lu\n", world_rank, e.from, info.label, e.to);
			} else if (rto == world_rank && rfrom == world_rank) {
				if (local_labels[e.from - offset]
						< local_labels[e.to - offset]) {
					local_labels[e.to - offset] = local_labels[e.from - offset];
					label_updates[e.to - offset] = 1;
					converged = 0;
					//printf("accel: update from %d(%d) to %d(%d)\n", e.to, local_labels[e.to - offset], e.from, local_labels[e.from - offset]);
				}
			} else {
				// edge source vertex doesn't belong to us
			}
		}

		// send(output_size = count)
		ctrl->output_size = count;
		count = 0;

		// send converged signal
		ctrl->converged = converged;

		// signal begin commuications
		ctrl->output_valid = 1;


		// wait for cpu to finish
		// read for iteration complete packet
		while (!(ctrl->input_valid)) {
			;
		}

		converged = 1;


		// process incoming data
		size_t input_size = ctrl->input_size;
		for (size_t i = 0; i < input_size; i++) {
			info_t info = input[i];

			//printf("accel: recv to %d label %d\n", info.to, info.label);
			if (local_labels[info.to - offset] != info.label) {
				local_labels[info.to - offset] = info.label;
				label_updates[info.to - offset] = 1;
				converged = 0;
			}
		}

		/* printf("top processed incoming data\n"); */
		ctrl->input_valid = 0;

		// wait till send is done on the proc end
		while (ctrl->output_valid) {
			;
		}

		if (ctrl->done) {
			break;
		}
	}

	for (int i = offset;
			i < rtov_upper(world_rank, world_size, num_vertices); i++) {
#pragma HLS UNROLL factor=16
		//printf("labels %d",local_labels[i - offset] );
		labels[i] = local_labels[i - offset];
		if(local_labels[i - offset]!= 0) {

			printf("[%i]: %lu ", world_rank, local_labels[i - offset]);
		}
	}
	printf("offset %d ", offset);
	printf("upper %d\n", rtov_upper(world_rank, world_size, num_vertices));
	return;
}
