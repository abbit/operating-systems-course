#include "list.h"

#include <stdlib.h>

#include "sflib/threads.h"

pthread_mutexattr_t *list_mtx_attr = NULL;

List *List_new(void) {
  List *list = malloc(sizeof(List));
  if (list == NULL) return NULL;

  list->head = NULL;

  if (list_mtx_attr == NULL) {
    list_mtx_attr = malloc(sizeof(pthread_mutexattr_t));
    pthread_mutexattr_init(list_mtx_attr);
    pthread_mutexattr_settype(list_mtx_attr, PTHREAD_MUTEX_ERRORCHECK);
  }

  list->head_mtx = sf_mtx_create(list_mtx_attr);

  return list;
}

void List_empty(List *list) {
  ListNode *current = list->head;
  ListNode *next;
  while (current != NULL) {
    next = current->next;
    free(current->value);
    sf_mtx_destroy(&current->mtx);
    free(current);
    current = next;
  }
  list->head = NULL;
}

void List_free(List *list) {
  List_empty(list);
  sf_mtx_destroy(&list->head_mtx);
  pthread_mutexattr_destroy(list_mtx_attr);
  free(list_mtx_attr);
  free(list);
}

List *List_pushFront(List *list, char *value) {
  ListNode *node = malloc(sizeof(ListNode));
  if (node == NULL) return NULL;

  node->value = value;
  node->mtx = sf_mtx_create(list_mtx_attr);
  sf_mtx_lock(list->head_mtx);
  if (list->head == 0) {
    node->next = NULL;
    list->head = node;
  } else {
    node->next = list->head;
    list->head = node;
  }
  sf_mtx_unlock(list->head_mtx);

  return list;
}

void swap(char **a, char **b) {
  char *tmp = *a;
  *a = *b;
  *b = tmp;
}

void List_sort(List *list, unsigned long sleep_time, int id) {
  sf_mtx_lock(list->head_mtx);
  if (list->head == NULL) {
    sf_mtx_unlock(list->head_mtx);
    return;
  }
  sf_mtx_unlock(list->head_mtx);

  int is_sorted = 1;
  ListNode *end = NULL;

  do {
    is_sorted = 1;
    sf_mtx_lock(list->head_mtx);
    sf_mtx_lock(list->head->mtx);
    ListNode *cur = list->head;
    sf_mtx_unlock(list->head_mtx);
    printf("- Thread %d started sorting TRAVERSAL\n", id);
    while (cur->next != end) {
      sf_mtx_lock(cur->next->mtx);
      printf("---- Thread %d started sorting STEP\n", id);
      if (strcmp(cur->value, cur->next->value) > 0) {
        swap(&cur->value, &cur->next->value);
        printf("----Thread %d swapped strings \"%s\" and \"%s\"\n", id,
               cur->next->value, cur->value);
        is_sorted = 0;
      }
      sf_mtx_unlock(cur->mtx);
      cur = cur->next;

      printf("---- Thread %d has done sorting STEP\n", id);
      usleep(sleep_time);
    }
    sf_mtx_unlock(cur->mtx);
    end = cur;
    printf("- Thread %d has done sorting TRAVERSAL\n", id);
  } while (!is_sorted);
}

void List_print(List *list) {
  // sf_mtx_lock(list->head_mtx);
  if (list->head == NULL) {
    // sf_mtx_unlock(list->head_mtx);
    return;
  }
  printf("Current list nodes:\n");
  // sf_mtx_lock(list->head->mtx);
  ListNode *cur = list->head;
  // sf_mtx_unlock(list->head_mtx);
  while (cur != NULL) {
    printf("%s\n", cur->value);
    // sf_mtx_unlock(cur->mtx);
    cur = cur->next;
    if (cur != NULL) {
      // sf_mtx_lock(cur->mtx);
    }
  }
  printf("\n");
}
