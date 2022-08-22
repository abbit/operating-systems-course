#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// will be done not more, than TOTAL_STEPS
#define TOTAL_STEPS 1000000000

typedef struct {
  long steps;
  long offset;
} ThreadParams;

double calc_pi_part(long offset, long steps) {
  double pi = 0;

  long end = offset + steps;
  for (long i = offset; i < end; i++) {
    pi += 1.0 / (i * 4.0 + 1.0);
    pi -= 1.0 / (i * 4.0 + 3.0);
  }

  return pi;
}

void *thread_func(void *params_void) {
  ThreadParams *params = (ThreadParams *)params_void;

  double *res = calloc(1, sizeof(double));
  *res = calc_pi_part(params->offset, params->steps);

  pthread_exit(res);
}

int main(int argc, char **argv) {
  int err;

  if (argc != 2) {
    fprintf(stderr,
            "Wrong number of arguments!\n"
            "Usage: ./compiled_lab8 [number of threads]\n");
    exit(EXIT_FAILURE);
  }

  int threads_num = 1;
  if ((threads_num = atoi(argv[1])) <= 0) {
    fprintf(stderr, "Number of threads must be positive!\n");
    exit(EXIT_FAILURE);
  }

  pthread_t *tid = malloc(threads_num * sizeof(pthread_t));

  long steps_per_thread = TOTAL_STEPS / threads_num;

  ThreadParams **threads_params = malloc(threads_num * sizeof(ThreadParams *));
  for (int i = 0; i < threads_num; i++) {
    threads_params[i] = malloc(sizeof(ThreadParams));
    threads_params[i]->steps = steps_per_thread;
    threads_params[i]->offset = i * steps_per_thread;
  }

  for (int i = 0; i < threads_num; i++) {
    if ((err = pthread_create(&tid[i], NULL, thread_func, threads_params[i]))) {
      fprintf(stderr, "pthread_create error: (%d)%s\n", err, strerror(err));
      exit(EXIT_FAILURE);
    }
  }

  double calculated_pi = 0;
  for (int i = 0; i < threads_num; i++) {
    void *calculated_pi_part = NULL;
    if ((err = pthread_join(tid[i], &calculated_pi_part))) {
      fprintf(stderr, "pthread_join error: (%d)%s\n", err, strerror(err));
      exit(EXIT_FAILURE);
    }
    calculated_pi += *(double *)calculated_pi_part;
    free(calculated_pi_part);
    free(threads_params[i]);
    threads_params[i] = NULL;
  }
  free(threads_params);
  threads_params = NULL;
  calculated_pi *= 4.0;

  printf("pi done - %.15g \n", calculated_pi);

  return EXIT_SUCCESS;
}
