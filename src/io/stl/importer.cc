#include <filesystem>
#include <fstream>

#include "ascii.hh"
#include "binary.hh"
#include "importer.hh"

namespace mp::io::stl
{
    void read_stl(TriMesh &mesh, const char *filepath)
    {
        std::ifstream ifs(filepath, std::ios::binary);

        /* Detect STL file type by comparing file size with expected file size,
         * could check if file starts with "solid", but some files do not adhere.
         */
        uint32_t tris_num = 0;
        auto file_size = std::filesystem::file_size(filepath);
        ifs.seekg(BINARY_HEADER_SIZE, std::ios_base::beg);
        ifs.read(reinterpret_cast<char *>(&tris_num), sizeof(uint32_t));
        auto expected_binary_file_size = BINARY_HEADER_SIZE + 4 + BINARY_STRIDE * tris_num;
        bool is_binary_stl = (file_size == expected_binary_file_size);

        if (is_binary_stl)
        {
            read_stl_binary(mesh, ifs);
        }
        else
        {
            read_stl_ascii(mesh, ifs);
        }
    }
}