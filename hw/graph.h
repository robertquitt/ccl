#include<stdlib.h>
#include<stdint.h>
#include<stdio.h>
#include<stdbool.h>
#ifndef CSIM
#include "hls_stream.h"
#include "ap_int.h"
#endif

#ifdef CSIM
#define bit_t bool
#define label_t uint64_t
#else
#define bit_t ap_uint<1>
#define label_t ap_uint<64>
#endif


#define MAX_EDGES 262144
#define MAX_VERTICES 16384

typedef struct ctrl {
	bit_t done; // input from CPU
	bit_t converged;
	bit_t output_valid;
	bit_t input_valid;
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

void top (ctrl_t* ctrl, edge_t* edges, info_t* input, info_t* output, label_t* labels,
		int world_rank, int world_size, int num_edges, int num_vertices);
