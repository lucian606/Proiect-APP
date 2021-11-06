#!/usr/bin/env python3
import os
import subprocess

def run_make():
    p = subprocess.run('make')
    if p.returncode != 0:
        print('Make failed')
        exit(1)

def run_non_mpi(args):
    p = os.popen(f'./ranksort {args}')
    ret = p.read()
    return ret

def run_mpi(args):
    p = os.popen(f'	mpirun --oversubscribe -np 5 ranksort {args}')
    ret = p.read()
    return ret    
    
def test_non_mpi(args):
    print(f"TESTING {args}")
    print("================")
    os.chdir(f'./{args}')
    run_make()

    for i in range(0, 20):
        out = run_non_mpi(f'../tests/inputs/in{i}.txt')
        result, time = out.split(' \n')
        result += ' \n'
        if result != refs[i]:
            print(f'Test {args} {i}: FAILED\n{time}')
        else:
            print(f'Test {args} {i}: PASSED\n{time}')

    print()
    os.chdir('..')

def test_mpi(args):
    print(f"TESTING {args}")
    print("================")
    os.chdir(f'./{args}')
    run_make()

    for i in range(0, 20):
        out = run_mpi(f'../tests/inputs/in{i}.txt')
        result, time = out.split(' \n')
        result += ' \n'
        if result != refs[i]:
            print(f'Test {args} {i}: FAILED\n{time}')
        else:
            print(f'Test {args} {i}: PASSED\n{time}')

    os.chdir('..')    

refs = []
for ref in range(0, 20):
    ref_file = f'./tests/refs/ref{ref}.txt'
    with open(ref_file, 'r') as f:
        refs.append(f.readline())

test_non_mpi('Pthreads')
test_non_mpi('OpenMP')
test_mpi('MPI')
test_mpi('MPI-Pthreads')
test_mpi('MPI-OpenMP')