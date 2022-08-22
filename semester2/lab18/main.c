#include <strings.h>

#include "list.h"
#include "sflib/threads.h"

#define BUFFER_SIZE 1024
#define SORTING_WORKER_SLEEP_DURATION 5

int worker_finish = 0;

void *sorting_worker(void *arg) {
  List *list = (List *)arg;

  while (!worker_finish) {
    sleep(SORTING_WORKER_SLEEP_DURATION);
    List_sort(list);
    printf("List sorted\n");
  }

  pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  int err;

  List *list = List_new();
  pthread_t tid = sf_pth_create(NULL, sorting_worker, (void *)list);

  char *buf = calloc(BUFFER_SIZE, sizeof(char));
  while (fgets(buf, BUFFER_SIZE, stdin)) {
    if (strcmp(buf, "\n")) {
      char *str = calloc(BUFFER_SIZE, sizeof(char));
      strncpy(str, buf, BUFFER_SIZE);
      List_pushFront(list, str);
    } else {
      List_print(list);
    }
    memset(buf, 0, BUFFER_SIZE);
  }
  worker_finish = 1;
  sf_pth_join(tid, NULL);
  free(buf);
  List_free(list);

  return EXIT_SUCCESS;
}
