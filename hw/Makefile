CPPFLAGS += -DCSIM -pthread -g -Wall -Wpedantic -Wno-unknown-pragmas

testbench: graph.cpp testbench.cpp graph.h

run: testbench facebook_combined.txt
	./testbench -u facebook_combined.txt

facebook_combined.txt:
	wget https://snap.stanford.edu/data/facebook_combined.txt.gz -O - | gunzip > $@

clean:
	rm -f testbench testbench.o graph.o

.PHONY: run clean
