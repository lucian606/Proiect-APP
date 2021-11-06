#!/usr/bin/env python3
from random import randint

dimmensions = [1000, 5000, 10000, 20000, 50000]
thread_counts = [4, 6, 8, 12]

def generate_array(n, bound):
    """
    Generates a test with n random integers.
    """
    test = []
    for _ in range(n):
        test.append(randint(0, bound))
    return test


for test_count in range(0, 20, 4):
    dimmension = dimmensions[test_count // 4]
    bound = dimmension * 10
    if test_count // 4 == 2:
        bound = dimmension // 100
    test_case = generate_array(dimmension, bound)
    ref = list(test_case)
    ref.sort()

    for i in range(0, 4):
        test_file = f'./tests/inputs/in{test_count + i}.txt'
        ref_file = f'./tests/refs/ref{test_count + i}.txt'
        thread_count = thread_counts[i]
        with open(test_file, 'w') as f:
            f.write(f'{dimmension}\n')
            f.write(f'{thread_count}\n')
            for n in test_case:
                f.write(f'{n} ')
        with open(ref_file, 'w') as f:
            for n in ref:
                f.write(f'{n} ')
            f.write('\n')