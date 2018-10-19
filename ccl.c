#include<stdint.h>
#include<stdio.h>

#define MAX_EDGES 1024
#define MAX_VERTICES 1024

typedef size_t vid_t;
typedef uint64_t pid_t;

struct edge {
	vid_t from;
	vid_t to;
};

static pid_t partition[MAX_VERTICES];
static struct edge edges[MAX_EDGES];
static size_t num_vertices

void main() {
	printf("Hello, World!\n");
	size_t i;
	for (i = 0; i < MAX_EDGES; i++) {
		struct edge e = edges[i];
	}
}

void load_graph() {
	scanf("%u", &num_vertices);
	size_t i;
	for (i = 0; i < num_vertices; i++) {
		scanf("%u %u");
	}
}
