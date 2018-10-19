.PHONY: default run

default: ccl

run: ccl k10.txt
	cat k10.txt > ccl

k10.txt: random_graph.py
	python3 random_graph.py 10 1 > $@
