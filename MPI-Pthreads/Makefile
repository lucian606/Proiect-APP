build:
	mpicc ranksort.c -o ranksort -lm -lpthread
run:
	mpirun --oversubscribe -np 5 ranksort ../tests/inputs/in0.txt
clear:
	rm ranksort
