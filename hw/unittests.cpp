#include "graph.h"
#include "assert.h"

int main() {
	assert(rtov_lower(0, 2, 100) == 0);
	assert(rtov_upper(0, 2, 100) == 50);
	assert(rtov_lower(1, 2, 100) == 50);
	assert(rtov_upper(1, 2, 100) == 100);

	assert(vtor( 0, 2, 100) == 0);
	assert(vtor(49, 2, 100) == 0);
	assert(vtor(50, 2, 100) == 1);
	assert(vtor(99, 2, 100) == 1);

	printf("%i\n", rtov_lower(0, 2, 4039));
	printf("%i\n", rtov_upper(0, 2, 4039));
	printf("%i\n", rtov_lower(1, 2, 4039));
	printf("%i\n", rtov_upper(1, 2, 4039));

	printf("%i\n", rtov_lower(0, 3, 4039));
	printf("%i\n", rtov_upper(0, 3, 4039));
	printf("%i\n", rtov_lower(1, 3, 4039));
	printf("%i\n", rtov_upper(1, 3, 4039));
	printf("%i\n", rtov_lower(2, 3, 4039));
	printf("%i\n", rtov_upper(2, 3, 4039));

	printf("%i\n", rtov_lower(0, 4, 4039));
	printf("%i\n", rtov_upper(0, 4, 4039));
	printf("%i\n", rtov_lower(1, 4, 4039));
	printf("%i\n", rtov_upper(1, 4, 4039));
	printf("%i\n", rtov_lower(2, 4, 4039));
	printf("%i\n", rtov_upper(2, 4, 4039));
	printf("%i\n", rtov_lower(3, 4, 4039));
	printf("%i\n", rtov_upper(3, 4, 4039));

	assert(rtov_lower(0, 2, 4039) == 0);
	assert(rtov_upper(0, 2, 4039) == rtov_lower(1, 2, 4039));
	assert(rtov_upper(1, 2, 4039) == 4039);

	int num_vertices = 4039;

	for (int n = 1; n < 100; n++) {
		for (int r = 0; r < n; r++) {
			int lower = rtov_lower(r, n, num_vertices);
			int upper = rtov_upper(r, n, num_vertices);
			for (int i = lower; i < upper; i++) {
				if (vtor(i, n, num_vertices) != r)
					printf("i = %i, n = %i\n", i, n);
				assert(vtor(i, n, num_vertices) == r);
			}
		}
	}


	return 0;
}
