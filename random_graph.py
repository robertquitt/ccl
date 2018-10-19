# import numpy as np
import argparse
import random
from itertools import combinations

def random_graph(n, p):
    random.seed(1)
    for a, b in combinations(range(n), 2):
        if random.random() < p:
            print(a, b)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('n')
    parser.add_argument('p')
    args = parser.parse_args()
    n = int(args.n)
    p = float(args.p)
    assert n > 0
    assert 0 <= p <= 1

    print(n)
    random_graph(n, p)

if __name__ == '__main__':
    main()
