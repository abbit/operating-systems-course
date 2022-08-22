#include <strings.h>

#include "sflib/list.h"
#include "sflib/threads.h"

#define BUFFER_SIZE 1024
#define SORTING_WORKER_SLEEP_DURATION 5

int worker_finish = 0;
pthread_mutex_t *mtx;

void printList(List *list) {
  printf("Current list nodes:\n");
  ListNode *cur = list->head;
  while (cur != NULL) {
    printf("%s", (char *)cur->value);
    cur = cur->next;
  }
  printf("\n");
}

void *sorting_worker(void *arg) {
  List *list = (List *)arg;

  while (!worker_finish) {
    sleep(SORTING_WORKER_SLEEP_DURATION);
    sf_mtx_lock(mtx);
    List_sort(list, strcmp, sizeof(char *));
    sf_mtx_unlock(mtx);
    printf("List sorted\n");
  }

  pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  int err;

  List *list = List_new(free);
  mtx = sf_mtx_create(NULL);
  pthread_t tid = sf_pth_create(NULL, sorting_worker, (void *)list);

  char *buf = calloc(BUFFER_SIZE, sizeof(char));
  while (fgets(buf, BUFFER_SIZE, stdin)) {
    sf_mtx_lock(mtx);
    if (strcmp(buf, "\n")) {
      char *str = calloc(BUFFER_SIZE, sizeof(char));
      strncpy(str, buf, BUFFER_SIZE);
      List_pushFront(list, str);
    } else {
      printList(list);
    }
    sf_mtx_unlock(mtx);
    memset(buf, 0, BUFFER_SIZE);
  }
  worker_finish = 1;
  sf_pth_join(tid, NULL);
  sf_mtx_destroy(mtx);
  free(buf);
  List_free(list);

  return EXIT_SUCCESS;
}
