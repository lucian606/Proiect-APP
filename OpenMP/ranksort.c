#include <stdio.h>
#include <omp.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

int *v;
int *u;
int *ranks;
int N;
int P;

omp_lock_t *locks;

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
    fscanf(f, "%d", &P);    
    v = (int*) malloc(sizeof(int) * N + 1);
	u = (int*) malloc(sizeof(int) * N + 1);
    ranks = (int*) calloc(N + 1, sizeof(int)); 
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
    int i, j, id;
    readInput(argv[1]);
    omp_set_num_threads(P);
    struct timeval start_t;
    struct timeval end_t;
    locks = (omp_lock_t *) malloc(sizeof(omp_lock_t) * N + 1);
    gettimeofday(&start_t, NULL);
    for (int i = 0; i < N; i++) {
        omp_init_lock(&locks[i]);
    }
    #pragma omp parallel for private(i, j) shared(u, v, ranks)
    for (i = 0; i < N; i++) {
        for (int j = 0; j < i; j++) {
            if (v[i] >= v[j]) {
                omp_set_lock(&locks[i]);
                ranks[i] += 1;
                omp_unset_lock(&locks[i]);
            } else {
                omp_set_lock(&locks[j]);
                ranks[j] += 1;
                omp_unset_lock(&locks[j]);
            }
        }
    }

    for (int j = 0; j < N; j++) {
        u[ranks[j]] = v[j];
    }

    for (int i = 0; i < N; i++) {
        omp_destroy_lock(&locks[i]);
    }

    gettimeofday(&end_t, NULL);
    double time_taken = time_diff(&start_t, &end_t);
    displayVector(u);
    printf("Time taken: %.4lf seconds\n", time_taken);
    free(locks);
    free(v);
    free(u);
    free(ranks);
    return 0;
}