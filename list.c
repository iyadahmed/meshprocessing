#include <stdlib.h>

#include "list.h"

void prepend(List **head_ref, void *data) {
  List *new_elem = malloc(sizeof(List));
  if (!new_elem) {
    return;
  }
  new_elem->data = data;
  new_elem->next = *head_ref;
  *head_ref = new_elem;
}

List *find(List *head, void *data) {
  while (head) {
    if (head->data == data) {
      return head;
    }
    head = head->next;
  }
  return NULL;
}