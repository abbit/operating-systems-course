#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include <pthread.h>
#include <stddef.h>

typedef struct list_node {
  struct list_node *next;
  char *value;
  pthread_mutex_t *mtx;
} ListNode;

typedef struct {
  ListNode *head;
  pthread_mutex_t *head_mtx;
} List;

List *List_new(void);
void List_empty(List *list);
void List_free(List *list);
List *List_push(List *list, char *value);
List *List_pushFront(List *list, char *value);
void List_sort(List *list);
void List_print(List *list);

#endif  // LINKED_LIST_H_
