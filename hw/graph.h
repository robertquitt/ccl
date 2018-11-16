#include<stdint.h>
#include<stdio.h>
#include<stdbool.h>
#include "hls_stream.h"
#include "ap_int.h"

#define MAX_EDGES 131072
#define MAX_VERTICES 8192

typedef struct ctrl {
	ap_uint<1> done; // input from CPU
	ap_uint<1> converged;
	ap_uint<1> output_valid;
	ap_uint<1> input_valid;
	size_t output_size;
	size_t input_size;
} ctrl_t;


typedef struct edge {
    uint64_t from;
    uint64_t to;
} edge_t;

typedef struct info {
	uint64_t to;
	uint64_t label;
} info_t;


void top (ctrl_t* ctrl, edge_t* edges, info_t* input, info_t* output, ap_uint<64>* labels,
		int world_rank, int world_size, int num_edges, int num_vertices);
