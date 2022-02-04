#include <stdlib.h>

#include "list.h"

void list_prepend(List **head_ref, void *data) {
  List *new_elem = malloc(sizeof(List));
  if (NULL == new_elem) {
    return;
  }
  new_elem->data = data;
  new_elem->next = *head_ref;
  *head_ref = new_elem;
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

void list_find_remove(List **head_ref, void *data) {
  if ((*head_ref)->data == data) {
    *head_ref = (*head_ref)->next;
    return;
  }
  List *head = *head_ref;
  while (head) {
    if (NULL != head->next) {
      if (head->next->data == data) {
        free(head->next);
        head->next = head->next->next;
        return;
      }
    }
    head = head->next;
  }
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