#include <stdlib.h>
#include <string.h>

#include "mesh.h"

/* TODO: preallocate to prevent a million call to malloc */

void prepend_triangle(Triangle **head_ref, const Triangle *tri) {
  Triangle *new_tri = malloc(sizeof(Triangle));
  if (new_tri == NULL) {
    return;
  }
  memcpy(new_tri, tri, sizeof(Triangle));
  new_tri->next = *head_ref;
  *head_ref = new_tri;
}

void free_triangles(Triangle **head_ref) {
  Triangle *current = *head_ref;
  Triangle *next = NULL;
  while (current != NULL) {
    next = current->next;
    free(current);
    current = next;
  }
  *head_ref = NULL;
}