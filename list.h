#ifndef LIST_H
#define LIST_H

typedef struct List {
  void *data;
  struct List *next;
} List;

void prepend(List **head_ref, void *data);
List *find(List *head, void *data);

#endif