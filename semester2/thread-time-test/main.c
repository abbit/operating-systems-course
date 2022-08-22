#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define THREADS 100

void *thread_func(void *param) {
  int num = *(int *)param;
  printf("Hello, %d!\n", num);

  pthread_exit(0);
}

int main(int argc, char **argv) {
  pthread_t tid[THREADS];
  pthread_attr_t tattr;
  int err;
  struct timespec start, end;

  pthread_attr_init(&tattr);

  int a = 5;
  clock_gettime(CLOCK_REALTIME, &start);
  for (int i = 0; i < THREADS; i++) {
    err = pthread_create(&tid[i], &tattr, thread_func, &a);
    if (err) {
      fprintf(stderr, "pthread_create error: (%d)%s\n", err, strerror(err));
      exit(EXIT_FAILURE);
    }
  }
  clock_gettime(CLOCK_REALTIME, &end);

  for (int i = 0; i < THREADS; i++) {
    if ((err = pthread_join(tid[i], NULL))) {
      fprintf(stderr, "pthread_cancel error: (%d)%s\n", err, strerror(err));
      exit(EXIT_FAILURE);
    }
  }

  double totalTime =
      end.tv_sec - start.tv_sec + 0.001 * (end.tv_nsec - start.tv_nsec);

  printf("Total time taken for %d threads: %lf micros.\n", THREADS, totalTime);
  printf("Average time for 1 thread: %lf micros.\n", totalTime / THREADS);

  return EXIT_SUCCESS;
}
