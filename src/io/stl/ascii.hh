#pragma once

#include <fstream>
#include <vector>

#include "importer.hh"

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

namespace mp::io::stl
{
    void read_stl_ascii(std::ifstream &ifs, std::vector<Triangle> &tris);
}
