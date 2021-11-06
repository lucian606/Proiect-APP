#include <mpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>

#define MASTER 0

int N;
int thread_count;
int *v;
int *pos;
int *u;
int *vQSort;
int *recv_pos;
int start;
int end;

pthread_mutex_t *mutexes;

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

void *rank_par(void *arg) {
    int id = *(int *) arg;
	int count = end - start;
    int thread_start = id * (double) count/thread_count + start;
    int x = (id + 1) * (double) count/thread_count + start;
    int thread_end = x < end ? x : end;
	for (int i = thread_start; i < thread_end; i++) {
        for (int j = 0; j < i; j++) {
            if (v[j] <= v[i]) {
                pthread_mutex_lock(&mutexes[i]);
                pos[i] += 1;
                pthread_mutex_unlock(&mutexes[i]);   
            } else {
                pthread_mutex_lock(&mutexes[j]);
                pos[j] += 1;
                pthread_mutex_unlock(&mutexes[j]);
            }
        }
    }

    pthread_exit(NULL);
}

void readInput(char *filename) {
    FILE *f = fopen(filename, "r");
    fscanf(f, "%d", &N);
    fscanf(f, "%d", &thread_count);
	v = (int*) malloc(sizeof(int)*N);
	pos = (int *) calloc(N, sizeof(int));
	recv_pos = (int *) calloc(N, sizeof(int));
	u = (int*)malloc(sizeof(int)*N);
	vQSort = (int*)malloc(sizeof(int)*N);
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
		vQSort = (int*)malloc(sizeof(int)*N);
	}
	MPI_Bcast(&thread_count, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(v, N, MPI_INT, MASTER, MPI_COMM_WORLD);
    // send the vector to all processes

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

		gettimeofday(&end_t, NULL);
		double time_take = time_diff(&start_t, &end_t);
		displayVector(u);
		printf("Time taken: %.4lf seconds\n", time_take);
	} else {
		
		int P = nProcesses - 1;
		start = ((rank - 1) * (double) N / P);
		end = min(rank * (double) N / P, N);
		mutexes = malloc(sizeof(pthread_mutex_t) * N);
		for (int i = 0; i < N; i++) {
			int r = pthread_mutex_init(&mutexes[i], NULL);
		}
		pthread_t *threads = malloc(sizeof(pthread_t) * thread_count);
		void *status;
		int r;
		long *arguments = malloc(sizeof(long) * thread_count);

		for (long id = 0; id < thread_count; id++) {
			arguments[id] = id;
			r = pthread_create(&threads[id], NULL, rank_par, &arguments[id]);
			if (r) {
				printf("Erroare la creare thread-ului %ld \n", id);
				exit(-1);
			}
		}

		for (long id = 0; id < thread_count; id++) {
			r = pthread_join(threads[id], &status);
			if (r) {
				printf("Erroare la asteptarea thread-ului %ld \n", id);
				exit(-1);
			}
		}

		for (int i = 0; i < N; i++) {
			pthread_mutex_destroy(&mutexes[i]);
		}
		// send the new positions to process MASTER
		MPI_Send(pos, N, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();
	return 0;
}
