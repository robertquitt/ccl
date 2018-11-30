# Must have openmpi installed (openmpi-bin on Ubuntu)

CPPFLAGS += -DCSIM -pthread -g -Wall -Wno-unknown-pragmas

EXECS = testbench mpirunner unittests

all: $(EXECS)

testbench: graph.cpp testbench.cpp

mpirunner: graph.cpp mpirunner.cpp
	mpic++ $(CPPFLAGS) $^ -o $@

unittests: graph.cpp unittests.cpp

test: unittests
	./unittests && echo unit tests passed

run: testbench facebook_combined.txt
	./testbench -u facebook_combined.txt > log
	head log
	tail log

run-cpu: testbench facebook_combined.txt
	./testbench -uc facebook_combined.txt > log
	head log
	tail log

run-mpi: mpirunner facebook_combined.txt
	mpirun -n 2 ./mpirunner

facebook_combined.txt:
	wget https://snap.stanford.edu/data/facebook_combined.txt.gz -O - | gunzip > $@

clean:
	rm -f $(EXECS)

.PHONY: run clean test
