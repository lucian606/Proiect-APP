#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

int *v;
int *u;
int *ranks;
int N;
int P;

pthread_mutex_t *mutexes;
pthread_barrier_t barrier;

void displayVector(int *v) {
	// DO NOT MODIFY
	int i;
	for(i = 0; i < N; i++) {
		printf("%d ", v[i]);
	}
	printf("\n");
}

void *rank_par(void *arg) {
    int id = (*(int *) arg);
    int start = id * (double) N/P;
    int x = (id + 1) * (double) N/P;
    int end = x < N ? x : N;

    for (int i = start; i < end; i++) {
        for (int j = 0; j < i; j++) {
            if (v[j] <= v[i]) {
                pthread_mutex_lock(&mutexes[i]);
                ranks[i] += 1;
                pthread_mutex_unlock(&mutexes[i]);   
            } else {
                pthread_mutex_lock(&mutexes[j]);
                ranks[j] += 1;
                pthread_mutex_unlock(&mutexes[j]);
            }
        }
    }

    pthread_exit(NULL);
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
    readInput(argv[1]);
    pthread_t *threads = malloc(sizeof(pthread_t) * P + 1);
    long *arguments = malloc(sizeof(long) * P + 1);
    mutexes = malloc(sizeof(pthread_mutex_t) * N + 1);
    void *status;
    int r;

    struct timeval start_t;
    struct timeval end_t;
    gettimeofday(&start_t, NULL);
    for (int i = 0; i < N; i++) {
        pthread_mutex_init(&mutexes[i], NULL);
    }
    pthread_barrier_init(&barrier, NULL, P);

    for (long id = 0; id < P; id++) {
        arguments[id] = id;
        r = pthread_create(&threads[id], NULL, rank_par, &arguments[id]);
        if (r) {
            printf("Erroare la creare thread-ului %ld \n", id);
            exit(-1);
        }
    }

    for (long id = 0; id < P; id++) {
        r = pthread_join(threads[id], &status);
        if (r) {
            printf("Erroare la asteptarea thread-ului %ld \n", id);
            exit(-1);
        }
    }

    for (int j = 0; j < N; j++) {
        u[ranks[j]] = v[j];
    }

    gettimeofday(&end_t, NULL);
    double time_taken = time_diff(&start_t, &end_t);
    displayVector(u);
    printf("Time taken: %.4lf seconds\n", time_taken);
    for (int i = 0; i < N; i++) {
        pthread_mutex_destroy(&mutexes[i]);
    }
	pthread_barrier_destroy(&barrier);
    free(threads);
    free(ranks);
    free(u);
    free(v);
    free(arguments);
    pthread_exit(NULL);
}