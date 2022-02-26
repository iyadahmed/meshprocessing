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

#include <cctype>
#include <cerrno>
#include <cstdbool>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "mesh.hpp"
#include "stl.hpp"

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

static void read_stl_binary(Mesh &mesh, FILE *file) {
  fseek(file, BINARY_HEADER, SEEK_SET);
  uint32_t num_tri = 0;
  if (fread(&num_tri, sizeof(uint32_t), 1, file) < 1) {
    return;
  }

  Triangle current_triangle{};

  /* TODO: switch num_tri endianess if machine is not little endian */
  for (int i = 0; i < num_tri; i++) {
    if (fread(&current_triangle, sizeof(Triangle), 1, file) == 0) {
      return;
    }
    mesh.triangles.push_back(current_triangle);

    /* Skip "Attribute byte count" */
    if (fseek(file, sizeof(uint16_t), SEEK_CUR) != 0) {
      return;
    }
  }
}

static int parse_float3_str(char *buf, float out[3]) {
  errno = 0;
  char *startptr = buf;
  char *endptr = NULL;
  for (int i = 0; i < 3; i++) {
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

static void read_stl_ascii(Mesh &mesh, FILE *file) {
  char line_buf[1024] = {0};
  char *line_stripped = NULL;
  char *facet_normal_str = NULL;
  char *vertex_location_str = NULL;
  Triangle current_triangle{};

  fseek(file, 0, SEEK_SET);
  /* Skip header line */
  if (fgets(line_buf, 1024, file) == NULL) {
    return;
  }
  while (fgets(line_buf, 1024, file) != NULL) {
    line_stripped = lstrip_unsafe(line_buf);
    if (strncmp(line_stripped, "facet", 5) == 0) {
      /* Skip "facet" */
      facet_normal_str = lstrip_token_unsafe(line_stripped);
      /* Skip "normal" */
      facet_normal_str = lstrip_token_unsafe(facet_normal_str);
      if (parse_float3_str(facet_normal_str, current_triangle.custom_normal) != 0) {
        return;
      }
    } else if (strncmp(line_stripped, "vertex", 6) == 0) {
      for (int i = 0; i < 3; i++) {
        /* Skip "vertex" */
        vertex_location_str = lstrip_token_unsafe(line_stripped);
        if (parse_float3_str(vertex_location_str, current_triangle.vertices[i]) != 0) {
          return;
        }
        if (fgets(line_buf, 1024, file) == NULL) {
          return;
        }
        line_stripped = lstrip_unsafe(line_buf);
      }
      mesh.triangles.push_back(current_triangle);
    }
  }
}

void read_stl(Mesh &mesh, char *filepath) {
  /* TODO: make fopen work for utf8 paths on Windows */
  FILE *file = fopen(filepath, "rb");
  if (file == NULL) {
    return;
  }
  /* TODO: check if STL is valid */
  if (is_ascii_stl(file)) {
    read_stl_ascii(mesh, file);
  } else {
    read_stl_binary(mesh, file);
  }
  fclose(file);
}