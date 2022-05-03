#include <cctype>
#include <cerrno>
#include <cstdbool>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdio.h>

#include "mesh.hpp"
#include "stl.hpp"

const size_t BINARY_HEADER = 80;
const size_t BINARY_STRIDE = 12 * 4 + 2;

static inline long calc_file_size(FILE *file)
{
  // TODO: error checking for fseek and ftell
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);
  return file_size;
}

class Buffer
{
private:
  char *m_mem = NULL;
  size_t m_size = 0;
  size_t m_current_location = 0;

public:
  Buffer(size_t size)
  {
    this->m_mem = new char[size];
    this->m_size = size;
    this->m_current_location = 0;
  }

  size_t location()
  {
    return m_current_location;
  }

  size_t size()
  {
    return m_size;
  }

  void printn(size_t n)
  {
    for (int i = 0; (i < n) && ((i + this->m_current_location) < this->m_size); i++)
    {
      putchar(this->m_mem[this->m_current_location + i]);
    }
  }

  static Buffer from_file(FILE *file)
  {
    // TODO: error checking for calc file size and fread
    long file_size = calc_file_size(file);
    Buffer buffer(file_size + 1);
    buffer.m_size = fread(buffer.m_mem, 1, file_size, file);
    buffer.m_mem[buffer.m_size] = '\0'; // Ensure null termination, for strtof to work
    return buffer;
  }

  void drop_leading_whitespace()
  {
    while (this->m_current_location < m_size)
    {
      if (!isspace(this->m_mem[this->m_current_location]))
      {
        break;
      }
      this->m_current_location++;
    }
  }

  void drop_leading_non_whitespace()
  {
    while (this->m_current_location < m_size)
    {
      if (isspace(this->m_mem[this->m_current_location]))
      {
        break;
      }
      this->m_current_location++;
    }
  }

  bool parse_token(const char *token, size_t max_size)
  {
    this->drop_leading_whitespace();
    size_t i = 0;
    for (; (i < max_size) && ((this->m_current_location + i) < this->m_size); i++)
    {
      if (this->m_mem[this->m_current_location + i] != token[i])
      {
        return false;
      }
    }
    this->m_current_location += i;
    return true;
  }

  bool parse_float(float *out)
  {
    errno = 0;
    char *endptr = NULL;
    *out = strtof(this->m_mem + this->m_current_location, &endptr);
    if ((errno != 0) || (endptr == (this->m_mem + this->m_current_location)))
    {
      return false;
    }
    this->m_current_location += (endptr - (this->m_mem + this->m_current_location));
    return true;
  }

  bool is_end()
  {
    return this->m_current_location >= this->m_size;
  }

  void free()
  {
    delete[] this->m_mem;
  }
};

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
    perror("STL Importer: WARNING! Reported size (facet number) is 0, assuming "
           "invalid binary STL file.");
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

static BinarySTLTriangle *read_stl_binary_core(FILE *file,
                                               size_t *num_read_tris)
{
  uint32_t num_tri = 0;
  fseek(file, BINARY_HEADER, SEEK_SET);
  if (fread(&num_tri, sizeof(uint32_t), 1, file) < 1)
  {
    perror("STL Importer: Failed to read binary STL triangle count.");
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

static inline void read_stl_ascii_vertex(Mesh &mesh, Buffer &buf)
{
  float float3_buf[3];
  for (int i = 0; i < 3; i++)
  {
    if (!buf.parse_float(float3_buf + i))
    {
      puts("STL Importer: ERROR! failed to parse float.");
      return;
    }
  }
  mesh.add_vertex(float3_buf[0], float3_buf[1], float3_buf[2]);
}

static void read_stl_ascii(Mesh &mesh, FILE *file)
{
  char tmp[1024];
  fseek(file, 0, SEEK_SET);
  fgets(tmp, 1024, file); // Skip header line

  auto buf = Buffer::from_file(file);
  while (!buf.is_end())
  {
    if (buf.parse_token("vertex", 6))
    {
      read_stl_ascii_vertex(mesh, buf);
    }
    else
    {
      buf.drop_leading_whitespace();
      buf.drop_leading_non_whitespace();
    }
  }
  buf.free();
}

void read_stl(Mesh &mesh, const char *filepath)
{
  /* TODO: support utf8 paths on Windows */
  FILE *file = fopen(filepath, "rb");

  if (file == NULL)
  {
    fprintf(stderr, "Error opening file: %s\n", filepath);
    return;
  }

  if (is_ascii_stl(file))
  {
    printf("Reading ASCII STL: %s\n", filepath);
    read_stl_ascii(mesh, file);
  }
  else
  {
    printf("Reading BINARY STL: %s\n", filepath);
    read_stl_binary(mesh, file);
  }
  fclose(file);
}