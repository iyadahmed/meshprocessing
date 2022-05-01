#include <cctype>
#include <cerrno>
#include <cstdbool>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <iostream>

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
static void read_stl_binary(Mesh &mesh, FILE *file) {
  float float3_buf[3];
  uint32_t num_tri = 0;

  fseek(file, BINARY_HEADER, SEEK_SET);
  if (fread(&num_tri, sizeof(uint32_t), 1, file) < 1) {
    return;
  }

  /* TODO: switch num_tri endianess if machine is not little endian */
  for (int i = 0; i < num_tri; i++) {
    if (fread(float3_buf, sizeof(float[3]), 1, file) == 0) {
      return;
    }

    mesh.add_vertex(float3_buf[0], float3_buf[1], float3_buf[2]);

    /* Skip "Attribute byte count" */
    if (fseek(file, sizeof(uint16_t), SEEK_CUR) != 0) {
      return;
    }
  }
}

static int parse_float3_str(const char *str, float out[3]) {
  errno = 0;
  char *startptr = (char *)str;
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

// Returns 0 on success, 1 on error
static inline int parse_float_str(const char *str, float *out) {
  errno = 0;
  char *endptr = NULL;
  *out = strtof(str, &endptr);
  if ((errno != 0) || (endptr == str)) {
    return 1;
  }
  return 0;
}

static char *lstrip_unsafe(const char *str) {
  char *str_stripped = (char *)str;
  while ((*str_stripped != '\0') && isspace(*str_stripped)) {
    str_stripped++;
  }
  return str_stripped;
}

static char *lstrip_token_unsafe(const char *str) {
  char *str_stripped = (char *)str;
  while ((*str_stripped != '\0') && isspace(*str_stripped)) {
    str_stripped++;
  }
  while ((*str_stripped != '\0') && !isspace(*str_stripped)) {
    str_stripped++;
  }
  return str_stripped;
}

// Returns 1 on success, 0 on error
static int read_token(FILE *file, char *buffer, size_t buffer_len) {
  int c;
  size_t i = 1; // star from 1, 0 is always read by first loop
  // Skip leading whitespaces
  while (((c = fgetc(file)) != EOF) && (i < buffer_len)) {
    if (!isspace(c))
    {
      buffer[0] = c;
      break;
    }
  }
  if (feof(file) || ferror(file) || (i >= buffer_len)) {
    return 0;
  }
  // Read until whitespace
  while (((c = fgetc(file)) != EOF) && (i < buffer_len)) {
    if (isspace(c))
    {
      buffer[i] = '\0';
      break;
    }
    buffer[i++] = (char)c;
  }
  return 1;
}

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
static void read_stl_ascii(Mesh &mesh, FILE *file) {
  char token_buf[1024];
  float float3_buf[3];
  bool error = false;

  fseek(file, 0, SEEK_SET);
  fgets(token_buf, 1024, file); // Skip header line

  while (read_token(file, token_buf, 1024)) {
    // std::cout << token_buf << std::endl;
    if (strncmp(token_buf, "facet", 5) == 0) {
      read_token(file, token_buf, 1024); // Skip "normal"
      for (int i = 0; i < 3; i++) {
        read_token(file, token_buf, 1024);
        // parse_float_str(token_buf, float3_buf + i);
      }
    } else if (strncmp(token_buf, "vertex", 6) == 0) {
      for (int i = 0; i < 3; i++) {
        read_token(file, token_buf, 1024);
        if (parse_float_str(token_buf, float3_buf + i)) {
          std::cerr << "Error parsing float\n";
          return;
        }
      }
      // std::cout << float3_buf[0];
      mesh.add_vertex(float3_buf[0], float3_buf[1], float3_buf[2]);
    }
  }
}

void read_stl(Mesh &mesh,const char *filepath) {
  /* TODO: support utf8 paths on Windows */
  FILE *file = fopen(filepath, "rb");

  if (file == NULL) {
    std::cerr << "Error opening file " << filepath << std::endl;
    return;
  }

  /* TODO: check if STL is valid */
  if (is_ascii_stl(file)) {
    std::cout << "Reading ASCII STL " << filepath << std::endl;
    read_stl_ascii(mesh, file);
  } else {
    std::cout << "Reading Binary STL " << filepath << std::endl;
    read_stl_binary(mesh, file);
  }
  fclose(file);
}