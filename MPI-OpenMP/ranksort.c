#include<omp.h>
#include<mpi/mpi.h>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<sys/time.h>

#define MASTER 0

int N;
int thread_count;
int *v;
int *pos;
int *u;
int *recv_pos;

omp_lock_t *locks;

int min(int a, int b) {
	int result = a < b ? a : b;
	return result;
}

void displayVector(int * v) {
	// DO NOT MODIFY
	int i;
	for(i = 0; i < N; i++) {
		printf("%d ", v[i]);
	}
	printf("\n");
}

void readInput(char *filename) {
    FILE *f = fopen(filename, "r");
    fscanf(f, "%d", &N);
    fscanf(f, "%d", &thread_count);
	v = (int*) malloc(sizeof(int)*N);
	pos = (int *) calloc(N, sizeof(int));
	recv_pos = (int *) calloc(N, sizeof(int));
	u = (int*)malloc(sizeof(int)*N);
    for (int i = 0; i < N; i++) {
        fscanf(f, "%d", &v[i]);
    }
    fclose(f);
}

float time_diff(struct timeval *start, struct timeval *end)
{
    return (end->tv_sec - start->tv_sec) + 1e-6*(end->tv_usec - start->tv_usec);
}
 
int main(int argc, char **argv) {
	int rank, i, j;
	int nProcesses;
	MPI_Init(&argc, &argv);
	int sorted = 0;
	struct timeval start_t;
	struct timeval end_t;

	for (i = 0; i < N; i++)
		pos[i] = 0;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

    if (rank == MASTER) {
		readInput(argv[1]);
        gettimeofday(&start_t, NULL);
    }

	MPI_Bcast(&N, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	if (rank != MASTER) {
		v = (int*) malloc(sizeof(int)*N);
		pos = (int *) calloc(N, sizeof(int));
		u = (int*)malloc(sizeof(int)*N);
	}
	MPI_Bcast(&thread_count, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(v, N, MPI_INT, MASTER, MPI_COMM_WORLD);

	if(rank == MASTER) {
		// sort the vector v		
		for(i = 1; i < nProcesses; i++) {
			MPI_Recv(recv_pos, N, MPI_INT, i, 0, MPI_COMM_WORLD, NULL);
			for (int j = 0; j < N; j++) {
				pos[j] += recv_pos[j];
			}
		}
		for (i = 0; i < N; i++)
			u[pos[i]] = v[i];
        // recv the new pozitions

		gettimeofday(&end_t, NULL);
		double time_taken = time_diff(&start_t, &end_t);
		displayVector(u);
		printf("Time taken: %.4lf seconds\n", time_taken);
	} else {
		
		int P = nProcesses - 1;
		int start = ((rank - 1) * (double) N / P);
		int end = min(rank * (double) N / P, N);
		int i, j;
		locks = malloc(sizeof(omp_lock_t) * N);
		for(int i = 0; i < N; i++) {
			omp_init_lock(&locks[i]);
		}
        // compute the positions
		omp_set_num_threads(thread_count);
    	#pragma omp parallel for private(i, j) shared(v, pos)
		for (i = start; i < end; i++) {
			for (j = 0; j < i; j++) {
				if (v[j] <= v[i]) {
					omp_set_lock(&locks[i]);	
					pos[i] += 1;
					omp_unset_lock(&locks[i]);
				} else {
					omp_set_lock(&locks[j]);
					pos[j] += 1;
					omp_unset_lock(&locks[j]);
				}
			}
		}

		for (int i = 0; i < N; i++) {
			omp_destroy_lock(&locks[i]);
		}

		// send the new positions to process MASTER
		MPI_Send(pos, N, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();
	return 0;
}
