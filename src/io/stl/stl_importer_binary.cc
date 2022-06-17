#include "stl_importer_binary.hh"
#include "stl_importer.hh"

namespace mp::io::stl
{
    void read_stl_binary(std::ifstream &ifs, std::vector<Triangle> &tris)
    {
        const int chunk_size = 1024;
        uint32_t tris_num = 0;

        ifs.seekg(BINARY_HEADER_SIZE, std::ios_base::beg);
        if (!ifs.read(reinterpret_cast<char *>(&tris_num), sizeof(uint32_t)))
        {
            return;
        }

        tris.reserve(tris_num);

        STLBinaryTriangle tris_buf[chunk_size];
        size_t read_tris_num;
        while ((read_tris_num = ifs.read(reinterpret_cast<char *>(tris_buf), sizeof(STLBinaryTriangle) * chunk_size).gcount() / sizeof(STLBinaryTriangle)))
        {
            for (size_t i = 0; i < read_tris_num; i++)
            {
                tris.push_back(tris_buf[i].tri);
            }
        }
    }
}
