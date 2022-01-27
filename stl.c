/*  Binary STL spec.:
 *   UINT8[80]    – Header                  - 80 bytes
 *   UINT32       – Number of triangles     - 4 bytes
 *   For each triangle                      - 50 bytes:
 *     REAL32[3]   – Normal vector          - 12 bytes
 *     REAL32[3]   – Vertex 1               - 12 bytes
 *     REAL32[3]   – Vertex 2               - 12 bytes
 *     REAL32[3]   – Vertex 3               - 12 bytes
 *     UINT16      – Attribute byte count   -  2 bytes
 */

/*  ASCII STL spec.:
 *  solid name
 *    facet normal ni nj nk
 *      outer loop
 *        vertex v1x v1y v1z
 *        vertex v2x v2y v2z
 *        vertex v3x v3y v3z
 *      endloop
 *    endfacet
 *    ...
 *  endsolid name
 */

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stl.h"

static void prepend_triangle(TriangleList **head_ref, Triangle *tri) {
  TriangleList *elem = malloc(sizeof(TriangleList));
  if (!elem) {
    return;
  }
  Triangle *data = malloc(sizeof(Triangle));
  if (!data) {
    return;
  }
  memcpy(data, tri, sizeof(Triangle));
  elem->data = data;
  elem->next = *head_ref;
  *head_ref = elem;
}

void free_triangles(TriangleList **head_ref) {
  TriangleList *head = *head_ref;
  TriangleList *tmp = NULL;
  while (head) {
    tmp = head;
    head = head->next;
    free(tmp->data);
    free(tmp);
  }
  *head_ref = NULL;
}

const size_t BINARY_HEADER = 80;
const size_t BINARY_STRIDE = 12 * 4 + 2;

static bool is_ascii_stl(FILE *file) {
  /* Could check for "solid", but some files don't adhere */
  fseek(file, BINARY_HEADER, SEEK_SET);
  uint32_t num_tri = 0;
  if (fread(&num_tri, sizeof(uint32_t), 1, file) == 0) {
    return 1;
  }
  if (num_tri == 0) {
    /* Number of triangles is 0, assume binary */
    return false;
  }
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);
  return (file_size != BINARY_HEADER + 4 + BINARY_STRIDE * num_tri);
}

static TriangleList *read_stl_binary(FILE *file) {
  Triangle current_triangle = {0};
  /* Could allocate a triangle array instead,
   * but for sake of keeping result of ASCII and Binary reader the same */
  TriangleList *triangle_list = NULL;

  fseek(file, BINARY_HEADER, SEEK_SET);
  uint32_t num_tri = 0;
  if (fread(&num_tri, sizeof(uint32_t), 1, file) < 1) {
    return NULL;
  }
  /* TODO: switch num_tri endianess if machine is not little endian */
  for (int i = 0; i < num_tri; i++) {
    if (fread(&current_triangle, sizeof(Triangle), 1, file) == 0) {
      return triangle_list;
    }
    prepend_triangle(&triangle_list, &current_triangle);
    /* TODO: switch floats endianess if machine is not little endian */
    /* Skip "Attribute byte count" */
    if (fseek(file, sizeof(uint16_t), SEEK_CUR) != 0) {
      return triangle_list;
    }
  }
  return triangle_list;
}

static int parse_float3_str(char *buf, float out[3]) {
  errno = 0;
  char *startptr = buf;
  char *endptr = NULL;
  for (int i = 0; i < 3; i++) {
    /* TODO: check for null termination */
    out[i] = strtof(startptr, &endptr);
    if ((errno != 0) || (endptr == startptr)) {
      return 1;
    }
    startptr = endptr;
  }
  return 0;
}

static char *lstrip_unsafe(char *str) {
  char *str_stripped = str;
  while ((*str_stripped != '\0') && isspace(*str_stripped)) {
    str_stripped++;
  }
  return str_stripped;
}

static char *lstrip_token_unsafe(char *str) {
  char *str_stripped = str;
  while ((*str_stripped != '\0') && isspace(*str_stripped)) {
    str_stripped++;
  }
  while ((*str_stripped != '\0') && !isspace(*str_stripped)) {
    str_stripped++;
  }
  return str_stripped;
}

static TriangleList *read_stl_ascii(FILE *file) {
  Triangle current_triangle = {0};
  TriangleList *triangle_list = NULL;

  char line_buf[1024] = {0};
  char *line_stripped = NULL;
  char *facet_normal_str = NULL;
  char *vertex_location_str = NULL;

  fseek(file, 0, SEEK_SET);
  /* Skip header line */
  if (fgets(line_buf, 1024, file) == NULL) {
    return NULL;
  }
  while (fgets(line_buf, 1024, file) != NULL) {
    line_stripped = lstrip_unsafe(line_buf);
    if (strncmp(line_stripped, "facet", 5) == 0) {
      /* Skip "facet" */
      facet_normal_str = lstrip_token_unsafe(line_stripped);
      /* Skip "normal" */
      facet_normal_str = lstrip_token_unsafe(facet_normal_str);
      if (parse_float3_str(facet_normal_str, current_triangle.normal) != 0) {
        return triangle_list;
      }
    } else if (strncmp(line_stripped, "vertex", 6) == 0) {
      for (int i = 0; i < 3; i++) {
        /* Skip "vertex" */
        vertex_location_str = lstrip_token_unsafe(line_stripped);
        if (parse_float3_str(vertex_location_str,
                             current_triangle.vertices[i]) != 0) {
          return triangle_list;
        }
        if (fgets(line_buf, 1024, file) == NULL) {
          return triangle_list;
        }
        line_stripped = lstrip_unsafe(line_buf);
      }
      prepend_triangle(&triangle_list, &current_triangle);
    }
  }
  return triangle_list;
}

TriangleList *read_stl(char *filepath) {
  /* TODO: make fopen work for utf8 paths on Windows */
  FILE *file = fopen(filepath, "rb");
  if (!file) {
    return NULL;
  }
  /* TODO: check if STL is valid */
  if (is_ascii_stl(file)) {
    return read_stl_ascii(file);
  } else {
    return read_stl_binary(file);
  }
  fclose(file);
}