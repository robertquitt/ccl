.PHONY: default run

CC = mpicc
CCL = ./ccl

default: ccl

run: ccl k10.txt
	cat k10.txt | $(CCL)

run1: ccl n10p0.1.txt
	cat n10p0.1.txt | $(CCL)

run2: ccl n100p0.1.txt
	cat n100p0.1.txt | $(CCL)

run3: ccl n100p0.1s.txt
	cat n100p0.1s.txt | $(CCL)

k10.txt: random_graph.py
	python3 random_graph.py 10 1 > $@

n10p0.1.txt: random_graph.py
	python3 random_graph.py 10 0.1 > $@

n100p0.1.txt: random_graph.py
	python3 random_graph.py 100 0.1 > $@

n100p0.1s.txt: random_graph.py
	python3 random_graph.py 100 0.1 --shuffle > $@

clean:
	rm k10.txt n10p0.1.txt n100p0.1.txt n100p0.1s.txt ccl
