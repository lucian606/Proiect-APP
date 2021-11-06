build:
	mpicc ranksort.c -o ranksort -lm -fopenmp
run:
	mpirun --oversubscribe -np 4 ranksort ../tests/inputs/in1.txt
clear:
	rm ranksort
