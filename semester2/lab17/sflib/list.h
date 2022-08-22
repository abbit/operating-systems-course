#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include <stddef.h>

typedef struct list_node {
  struct list_node *next;
  void *value;
} ListNode;

typedef struct {
  ListNode *head;
  ListNode *tail;
  void (*free)(void *ptr);
  unsigned long length;
} List;

List *List_new(void (*freeValue)(void *value));
void List_empty(List *list);
void List_free(List *list);
List *List_push(List *list, void *value);
List *List_pushFront(List *list, void *value);
void List_sort(List *list, int (*comparator)(const void *a, const void *b),
               size_t value_size);

#endif  // LINKED_LIST_H_
