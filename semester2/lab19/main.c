#include <string.h>

#include "list.h"
#include "sflib/threads.h"

#define BUFFER_SIZE 1024
#define SORTING_WORKER_SLEEP_DURATION 5 * 1000000
#define SORTING_STEP_SLEEP_DURATION 1 * 1000000
#define SORTING_THREADS_COUNT 2

volatile int should_workers_finish = 0;

typedef struct {
  int id;
  List *list;
  unsigned long thread_sleep_time;
  unsigned long step_sleep_time;
} SortingWorkerArgs;

void *sorting_worker(void *arg) {
  SortingWorkerArgs *args = (SortingWorkerArgs *)arg;
  while (!should_workers_finish) {
    usleep(args->thread_sleep_time);
    printf("Thread #%d started sorting\n", args->id);
    List_sort(args->list, args->step_sleep_time, args->id);
    printf("Thread #%d has finished sorting\n", args->id);
  }

  pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  int err;

  List *list = List_new();

  pthread_t tid[SORTING_THREADS_COUNT];
  SortingWorkerArgs args[SORTING_THREADS_COUNT];
  for (int i = 0; i < SORTING_THREADS_COUNT; ++i) {
    args[i].id = i;
    args[i].list = list;
    args[i].thread_sleep_time = SORTING_WORKER_SLEEP_DURATION;
    args[i].step_sleep_time = SORTING_STEP_SLEEP_DURATION;
    tid[i] = sf_pth_create(NULL, sorting_worker, (void *)&args[i]);
  }

  char buf[BUFFER_SIZE];
  while (fgets(buf, BUFFER_SIZE, stdin)) {
    if (strcmp(buf, "\n")) {
      char *str = calloc(BUFFER_SIZE, sizeof(char));
      strncpy(str, buf, BUFFER_SIZE);
      str[strcspn(str, "\n")] = 0;
      List_pushFront(list, str);
    } else {
      List_print(list);
    }
    memset(buf, 0, BUFFER_SIZE);
  }
  should_workers_finish = 1;
  for (int i = 0; i < SORTING_THREADS_COUNT; ++i) {
    sf_pth_join(tid[i], NULL);
  }
  List_free(list);

  return EXIT_SUCCESS;
}
