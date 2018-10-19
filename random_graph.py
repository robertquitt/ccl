# import numpy as np
import argparse
import random
from itertools import combinations

def random_graph(n, p, shuffle=False):
    random.seed(1)
    edges = 0
    iterator = combinations(range(n), 2)

    if shuffle:
        iterator = list(iterator)
        random.shuffle(iterator)

    for a, b in iterator:
        if random.random() < p:
            print(a, b)
            edges += 1

    return edges


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('n')
    parser.add_argument('p')
    parser.add_argument('--shuffle', action='store_true')
    args = parser.parse_args()
    n = int(args.n)
    p = float(args.p)

    assert n > 0
    assert 0 <= p <= 1

    print(n)
    random_graph(n, p, shuffle=args.shuffle)

if __name__ == '__main__':
    main()
