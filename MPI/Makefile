build:
	mpicc ranksort.c -o ranksort -lm
run:
	mpirun --oversubscribe -np 4 ranksort ../tests/inputs/in19.txt
clear:
	rm ranksort
