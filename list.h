#ifndef LIST_H
#define LIST_H

typedef void (*ListDataFreeFuncPointer)(void *);

typedef struct List {
  void *data;

  struct List *prev;
  struct List *next;
} List;

List *list_prepend(List **head_ref, void *data);
List *list_find(List *head, void *data);
void list_remove_item(List *item, ListDataFreeFuncPointer data_free_func);
void list_free(List **head_ref, ListDataFreeFuncPointer data_free_func);

#endif