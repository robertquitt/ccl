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

	assert(rtov_lower(0, 2, 4039) == 0);
	assert(rtov_upper(0, 2, 4039) == 2019);
	assert(rtov_lower(1, 2, 4039) == 2019);
	assert(rtov_upper(1, 2, 4039) == 4039);

	assert(vtor(   0, 2, 4039) == 0);
	assert(vtor(   1, 2, 4039) == 0);
	assert(vtor(2018, 2, 4039) == 0);
	assert(vtor(2019, 2, 4039) == 1);
	assert(vtor(4037, 2, 4039) == 1);
	assert(vtor(4038, 2, 4039) == 1);

	return 0;
}
