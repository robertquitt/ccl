#include<stdint.h>
#include<stdio.h>
#include<stdbool.h>
#include "hls_stream.h"
#include "ap_int.h"

#define MAX_EDGES 1024
#define MAX_VERTICES 1024

typedef struct edge {
    uint64_t from;
    uint64_t to;
} edge_t;


static uint64_t labels[MAX_VERTICES];
// Returns processor # 
static inline int vtor(int vertex_idx, int world_size, int num_vertices) {
    return (vertex_idx) * world_size / num_vertices;
}

static inline int rtov_upper(int rank, int world_size, int num_vertices) {
    return num_vertices * (rank + 1) / world_size;
}

static inline int rtov_lower(int rank, int world_size, int num_vertices) {
    return num_vertices * rank / world_size;
}

void top (edge* edges, edge* output, ap_uint<64>* labels, int output_size, int world_rank, int world_size, int num_edges, int num_vertices) {
#pragma HLS INTERFACE s_axilite port=return bundle=control
#pragma HLS INTERFACE s_axilite port=edges bundle=control
#pragma HLS INTERFACE m_axi port=edges offset=slave depth=128 bundle=gmem0
#pragma HLS INTERFACE s_axilite port=output bundle=control
#pragma HLS INTERFACE m_axi port=output offset=slave depth=128 bundle=gmem1
#pragma HLS INTERFACE s_axilite port=labels bundle=control
#pragma HLS INTERFACE m_axi port=labels offset=slave depth=128 bundle=gmem2

#pragma HLS INTERFACE s_axilite port=output_size bundle=control
#pragma HLS INTERFACE s_axilite port=world_rank bundle=control
#pragma HLS INTERFACE s_axilite port=world_size bundle=control
#pragma HLS INTERFACE s_axilite port=num_edges bundle=control
#pragma HLS INTERFACE s_axilite port=num_vertices bundle=control

    
    ap_uint<64> local_labels[MAX_VERTICES];
	#pragma HLS ARRAY_PARTITION variable=local_labels complete dim=0

    //edge_t output[MAX_EDGES];
 
    size_t i;
    int offset = rtov_lower(world_rank, world_size, num_vertices); 
    for (i = offset; i < rtov_upper(world_rank, world_size, num_vertices); i++) {
#pragma HLS UNROLL factor=16
    	local_labels[i-offset] = i;
    }
   
    int count = 0;
    for (i = 0; i < num_edges; i++) {
#pragma HLS UNROLL factor=16

        edge_t e = edges[i];

        int rto = vtor(e.to, world_size, num_vertices);
        int rfrom = vtor(e.from, world_size, num_vertices);

       if (rto != world_rank) {
            // write to other processor
            //MPI_Request send_request;
            //MPI_Isend(&e, 1, mpi_edge_t, rto, 0, MPI_COMM_WORLD, &send_request);
            output[count++] = e;
        } else {
            // edge touches two vertices in this rank
            local_labels[e.to - offset] = e.from;
        }
    }

    for (i = offset; i < rtov_upper(world_rank, world_size, num_vertices); i++) {
#pragma HLS UNROLL factor=16
        labels[i] = local_labels[i-offset];
    }
}
