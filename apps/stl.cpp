#include <cctype>
#include <cerrno>
#include <cstdbool>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdio.h>

#include "stl.hpp"
#include "mesh.hpp"

const size_t BINARY_HEADER = 80;
const size_t BINARY_STRIDE = 12 * 4 + 2;

static inline long calc_file_size(FILE *file)
{
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);
  return file_size;
}

/* Based on Blender's Python STL importer
 * https://github.com/blender/blender-addons/blob/599a8db33c45c2ad94f8d482f01b281252799770/io_mesh_stl/stl_utils.py#L62
 * Could check for "solid", but some files don't adhere */
static bool is_ascii_stl(FILE *file)
{
  fseek(file, BINARY_HEADER, SEEK_SET);
  uint32_t num_tri = 0;
  fread(&num_tri, sizeof(uint32_t), 1, file);
  if (num_tri == 0)
  {
    /* Number of triangles is 0, assume invalid binary */
    fputs("STL Importer: WARNING! Reported size (facet number) is 0, assuming invalid binary STL file.", stderr);
    return false;
  }
  long file_size = calc_file_size(file);
  return (file_size != BINARY_HEADER + 4 + BINARY_STRIDE * num_tri);
}

static inline void print_float(float value, int digit_count)
{
  char buf[128];
  gcvt(value, digit_count, buf);
  buf[127] = '\0';
  puts(buf);
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

#pragma pack(push, 1)
struct BinarySTLTriangle
{
  float normal[3];
  union
  {
    struct
    {
      float v1[3], v2[3], v3[3];
    };
    float verts[3][3];
  };
  uint16_t attribute_byte_count;
};
#pragma pack(pop)

static BinarySTLTriangle *read_stl_binary_core(FILE *file, size_t *num_read_tris)
{
  uint32_t num_tri = 0;
  fseek(file, BINARY_HEADER, SEEK_SET);
  if (fread(&num_tri, sizeof(uint32_t), 1, file) < 1)
  {
    fputs("STL Importer: Failed to read binary STL triangle count", stderr);
    return NULL;
  }
  auto tris = new BinarySTLTriangle[num_tri];
  *num_read_tris = fread(tris, sizeof(BinarySTLTriangle), num_tri, file);
  return tris;
}

static void read_stl_binary(Mesh &mesh, FILE *file)
{
  size_t num_tris;
  BinarySTLTriangle *tris = read_stl_binary_core(file, &num_tris);
  if (tris == NULL)
  {
    return;
  }
  mesh.reserve(num_tris * 3);
  for (size_t i = 0; i < num_tris; i++)
  {
    for (short j = 0; j < 3; j++)
    {
      auto v = tris[i].verts[j];
      // printf("Vertex: %f, %f, %f\n", v[0], v[1], v[2]);
      mesh.add_vertex(v[0], v[1], v[2]);
    }
  }
  delete[] tris;
}

static int parse_float3_str(const char *str, float out[3])
{
  errno = 0;
  char *startptr = (char *)str;
  char *endptr = NULL;
  for (int i = 0; i < 3; i++)
  {
    out[i] = strtof(startptr, &endptr);
    if ((errno != 0) || (endptr == startptr))
    {
      return 1;
    }
    startptr = endptr;
  }
  return 0;
}

/* Wrapper for strtof with error checking,
 * returns 0 on success, 1 on error */
static inline int parse_float_str(const char *str, float *out)
{
  errno = 0;
  char *endptr = NULL;
  *out = strtof(str, &endptr);
  if ((errno != 0) || (endptr == str))
  {
    return 1;
  }
  return 0;
}

static char *lstrip_unsafe(const char *str)
{
  char *str_stripped = (char *)str;
  while ((*str_stripped != '\0') && isspace(*str_stripped))
  {
    str_stripped++;
  }
  return str_stripped;
}

static char *lstrip_token_unsafe(const char *str)
{
  char *str_stripped = (char *)str;
  while ((*str_stripped != '\0') && isspace(*str_stripped))
  {
    str_stripped++;
  }
  while ((*str_stripped != '\0') && !isspace(*str_stripped))
  {
    str_stripped++;
  }
  return str_stripped;
}

// Returns 1 on success, 0 on error
static int read_token(FILE *file, char *buffer, size_t buffer_len)
{
  int c;
  size_t i = 1; // star from 1, 0 is always read by first loop
  // Skip leading whitespaces
  while (((c = fgetc(file)) != EOF) && (i < buffer_len))
  {
    if (!isspace(c))
    {
      buffer[0] = (char)c;
      break;
    }
  }
  if (feof(file) || ferror(file) || (i >= buffer_len))
  {
    return 0;
  }
  // Read until whitespace
  while (((c = fgetc(file)) != EOF) && (i < buffer_len))
  {
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

static inline void read_stl_ascii_vertex(Mesh &mesh, FILE *file)
{
  char token_buf[1024];
  float float3_buf[3];
  for (int i = 0; i < 3; i++)
  {
    read_token(file, token_buf, 1024);
    if (parse_float_str(token_buf, float3_buf + i))
    {
      fputs("STL Importer: ERROR! failed to parse float.", stderr);
      return;
    }
  }
  mesh.add_vertex(float3_buf[0], float3_buf[1], float3_buf[2]);
}

static inline void read_stl_ascii_facet(Mesh &mesh, FILE *file)
{
  char token_buf[1024];
  float float3_buf[3];
  read_token(file, token_buf, 1024); // Skip "normal"
  for (int i = 0; i < 3; i++)
  {
    read_token(file, token_buf, 1024);
    parse_float_str(token_buf, float3_buf + i);
  }
  // TODO: do something with the normal vector
}

static void read_stl_ascii(Mesh &mesh, FILE *file)
{
  char token_buf[1024];

  fseek(file, 0, SEEK_SET);
  fgets(token_buf, 1024, file); // Skip header line

  while (read_token(file, token_buf, 1024))
  {
    // if (memcmp(token_buf, "facet", 5) == 0)
    // {
    //   read_stl_ascii_facet(mesh, file);
    // }
    // else
    if (memcmp(token_buf, "vertex", 6) == 0)
    {
      read_stl_ascii_vertex(mesh, file);
    }
  }
}

void read_stl(Mesh &mesh, const char *filepath)
{
  /* TODO: support utf8 paths on Windows */
  FILE *file = fopen(filepath, "rb");

  if (file == NULL)
  {
    fputs("Error opening file: ", stderr);
    fputs(filepath, stderr);
    return;
  }

  /* TODO: check if STL is valid */
  if (is_ascii_stl(file))
  {
    puts("Reading ASCII STL: ");
    puts(filepath);
    read_stl_ascii(mesh, file);
  }
  else
  {
    puts("Reading Binary STL");
    puts(filepath);
    read_stl_binary(mesh, file);
  }
  fclose(file);
}