CPPFLAGS += -DCSIM -pthread -g -Wall -Wpedantic -Wno-unknown-pragmas

testbench: graph.cpp testbench.cpp graph.h

unittests: graph.cpp graph.h unittests.cpp

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

facebook_combined.txt:
	wget https://snap.stanford.edu/data/facebook_combined.txt.gz -O - | gunzip > $@

clean:
	rm -f testbench unittests

.PHONY: run clean test
