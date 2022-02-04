#include <stdlib.h>

#include "list.h"

List *list_prepend(List **head_ref, void *data) {
  List *new_elem = malloc(sizeof(List));
  if (NULL == new_elem) {
    return NULL;
  }
  List *head = *head_ref;
  if (NULL != head) {
    head->prev = new_elem;
  }
  new_elem->data = data;
  new_elem->next = head;
  new_elem->prev = NULL;
  *head_ref = new_elem;
  return new_elem;
}

List *list_find(List *head, void *data) {
  while (head) {
    if (head->data == data) {
      return head;
    }
    head = head->next;
  }
  return NULL;
}

/* Removes item and frees its memory */
void list_remove_item(List *item, ListDataFreeFuncPointer data_free_func) {
  if (NULL != item->prev) {
    item->prev->next = item->next;
  }
  if (NULL != item->next) {
    item->next->prev = item->prev;
  }
  if (NULL != data_free_func) {
    data_free_func(item->data);
  }
  free(item);
}

void list_free(List **head_ref, ListDataFreeFuncPointer data_free_func) {
  if (NULL == *head_ref) {
    return;
  }
  List *current_item = *head_ref;
  List *next_item = NULL;
  while (current_item) {
    next_item = current_item->next;
    if (NULL != data_free_func) {
      data_free_func(current_item->data);
    }
    free(current_item);
    current_item = next_item;
  }
  *head_ref = NULL;
}