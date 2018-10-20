.PHONY: default run

CC = mpicc
CCL = ./ccl
MPIRUN = mpirun -n 2

default: ccl

run: ccl k10.txt
	$(CCL) $(word 2,$^)

run1: ccl n10p0.1.txt
	$(CCL) $(word 2,$^)

run2: ccl n100p0.1.txt
	$(CCL) $(word 2,$^)

run3: ccl n100p0.1.shuf.txt
	$(CCL) $(word 2,$^)

run4: ccl n10p0.1.txt
	$(MPIRUN) $(CCL) $(word 2,$^)


k10.txt: random_graph.py
	python3 random_graph.py 10 1 > $@

n10p0.1.txt: random_graph.py
	python3 random_graph.py 10 0.1 > $@

n100p0.1.txt: random_graph.py
	python3 random_graph.py 100 0.1 > $@

n100p0.1.shuf.txt: n100p0.1.txt
	head -n 1 $< > $@
	tail -n +2 $< | shuf >> $@

clean:
	rm -f k10.txt n10p0.1.txt n100p0.1.txt n100p0.1s.txt ccl
