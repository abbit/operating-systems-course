#include <strings.h>

#include "sflib/threads.h"

#define WORKER_NAME_MAX_LENGTH 64
#define PRINT_TIMES 10

pthread_mutex_t *mtx;
pthread_cond_t *cond;
volatile int current_id = 0;

typedef struct {
  char name[WORKER_NAME_MAX_LENGTH];
  int id;
  int next_id;
} WorkerArgs;

WorkerArgs *WorkerArgs_create(char name[WORKER_NAME_MAX_LENGTH], int id,
                              int next_id) {
  WorkerArgs *args = malloc(sizeof(WorkerArgs));
  args->id = id;
  args->next_id = next_id;
  strncpy(args->name, name, WORKER_NAME_MAX_LENGTH);
  return args;
}

void *print_worker(void *arg) {
  WorkerArgs *args = (WorkerArgs *)arg;
  sf_mtx_lock(mtx);
  for (int i = 0; i < PRINT_TIMES; i++) {
    while (current_id != args->id) sf_cond_wait(cond, mtx);
    printf("line #%d from %s thread (id %d)\n", i + 1, args->name, args->id);
    current_id = args->next_id;
    sf_cond_broadcast(cond);
  }
  sf_mtx_unlock(mtx);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s THREADS_COUNT\n", argv[0]);
    return EXIT_FAILURE;
  }

  mtx = sf_mtx_create(NULL);
  cond = sf_cond_create(NULL);
  int threads_count = atoi(argv[1]);
  pthread_t *tid = malloc(sizeof(pthread_t) * threads_count);
  WorkerArgs **args = malloc(sizeof(WorkerArgs *) * (threads_count + 1));
  for (int i = 0; i < threads_count; ++i) {
    args[i] =
        WorkerArgs_create("created", i + 1, i + 2 > threads_count ? 0 : i + 2);
    tid[i] = sf_pth_create(NULL, print_worker, (void *)args[i]);
  }
  usleep(333);

  args[threads_count] = WorkerArgs_create("main", 0, 1);
  print_worker((void *)args[threads_count]);

  for (int i = 0; i < threads_count; ++i) {
    sf_pth_join(tid[i], NULL);
    free(args[i]);
  }
  free(args[threads_count]);
  free(tid);
  free(args);
  sf_mtx_destroy(&mtx);
  sf_cond_destroy(&cond);

  return EXIT_SUCCESS;
}
