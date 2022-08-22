#include "list.h"

#include <stdlib.h>

List *List_new(void (*freeValue)(void *value)) {
  List *list = malloc(sizeof(List));
  if (list == NULL) return NULL;

  list->head = list->tail = NULL;
  list->length = 0;
  list->free = freeValue;

  return list;
}

void List_empty(List *list) {
  ListNode *current = list->head;
  ListNode *next;
  unsigned long len = list->length;
  while (len--) {
    next = current->next;
    if (list->free) list->free(current->value);
    free(current);
    current = next;
  }
  list->head = list->tail = NULL;
  list->length = 0;
}

void List_free(List *list) {
  List_empty(list);
  free(list);
}

List *List_push(List *list, void *value) {
  ListNode *node = malloc(sizeof(ListNode));
  if (node == NULL) return NULL;

  node->value = value;
  node->next = NULL;
  if (list->length == 0) {
    list->head = list->tail = node;
  } else {
    list->tail->next = node;
    list->tail = node;
  }
  list->length++;

  return list;
}

List *List_pushFront(List *list, void *value) {
  ListNode *node = malloc(sizeof(ListNode));
  if (node == NULL) return NULL;

  node->value = value;
  if (list->length == 0) {
    node->next = NULL;
    list->head = list->tail = node;
  } else {
    node->next = list->head;
    list->head = node;
  }
  list->length++;

  return list;
}

void swap(void *ptr_a, void *ptr_b, size_t size) {
  char tmp;
  char *p_a = (char *)ptr_a;
  char *p_b = (char *)ptr_b;
  for (size_t i = 0; i < size; ++i) {
    tmp = p_a[i];
    p_a[i] = p_b[i];
    p_b[i] = tmp;
  }
}

void List_sort(List *list, int (*comparator)(const void *a, const void *b),
               size_t value_size) {
  int is_sorted = 1;
  ListNode *end = list->tail;

  do {
    is_sorted = 1;
    ListNode *cur = list->head;
    while (cur != end) {
      if (comparator(cur->value, cur->next->value) > 0) {
        swap(&cur->value, &cur->next->value, value_size);
        is_sorted = 0;
      }
      cur = cur->next;
    }
    end = cur;
  } while (!is_sorted);
}
