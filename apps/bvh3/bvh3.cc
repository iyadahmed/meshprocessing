#include <cmath>
#include <cstdint>
#include <iostream>
#include <algorithm>

#include "morton_codes.hh"
#include "stl_io.hh"
#include "vec3.hh"
#include "../timers.hh"

using namespace mp::io;

struct IDCodePair
{
    int triangle_id;
    unsigned int morton_code;
};

struct Node
{
    Node *child_a;
    Node *child_b;
    int primitive_id;
};

#define LeafNode(id) Node{nullptr, nullptr, id};
#define InternalNode(child_a, child_b) Node{child_a, child_b, -1};


int find_split(IDCodePair *sorted_id_code_pairs,
              int first,
              int last)
{
    // Identical Morton codes => split the range in the middle.

    unsigned int first_code = sorted_id_code_pairs[first].morton_code;
    unsigned int last_code = sorted_id_code_pairs[last].morton_code;

    if (first_code == last_code)
        return (first + last) >> 1;

    // Calculate the number of highest bits that are the same
    // for all objects, using the count-leading-zeros intrinsic.

    int common_prefix = clz(first_code ^ last_code);

    // Use binary search to find where the next bit differs.
    // Specifically, we are looking for the highest object that
    // shares more than commonPrefix bits with the first one.

    int split = first; // initial guess
    int step = last - first;

    do
    {
        step = (step + 1) >> 1;      // exponential decrease
        int new_split = split + step; // proposed new position

        if (new_split < last)
        {
            unsigned int splitCode = sorted_id_code_pairs[new_split].morton_code;
            int split_prefix = clz(first_code ^ splitCode);
            if (split_prefix > common_prefix)
                split = new_split; // accept proposal
        }
    } while (step > 1);

    return split;
}

Node *generate_hirearchy(
    Node *nodes,
    IDCodePair *sorted_id_code_pairs,
                         int first,
                         int last,
                         int num_used_nodes)
{

    // Single object => create a leaf node.

    if (first == last)
    {
        Node &node = nodes[num_used_nodes];
        node.child_a = nullptr;
        node.child_b = nullptr;
        node.primitive_id = sorted_id_code_pairs[first].triangle_id;
        num_used_nodes++;
        return &node;
    }

    // Determine where to split the range.

    int split = find_split(sorted_id_code_pairs, first, last);

    // Process the resulting sub-ranges recursively.

    Node *child_a = generate_hirearchy(nodes, sorted_id_code_pairs, first, split, num_used_nodes);
    Node *child_b = generate_hirearchy(nodes, sorted_id_code_pairs, split + 1, last, num_used_nodes);

    Node &internel_node = nodes[num_used_nodes];
    internel_node.child_a = child_a;
    internel_node.child_b = child_b;
    num_used_nodes++;
    return &internel_node;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        puts("Usage: bvh3 input.stl");
        return 1;
    }
    std::vector<stl::Triangle> tris;
    stl::read_stl(argv[1], tris);

    std::vector<IDCodePair> morton_codes(tris.size());

    Timer timer;
    #pragma omp parallel for
    for (int i =0; i < tris.size(); i++)
    {
        Vec3 *verts = (Vec3 *)tris[i].verts;
        Vec3 c = verts[0] + verts[1] + verts[2];
        morton_codes[i].triangle_id = i;
        morton_codes[i].morton_code = morton3D(c.x, c.y, c.z);
    }
    timer.tock("Calculating morton codes");

    auto sort_by_morton_code = [](const IDCodePair &a, const IDCodePair &b)
    {
        return a.morton_code < b.morton_code;
    };

    timer.tick();
    std::sort(morton_codes.begin(), morton_codes.end(), sort_by_morton_code);
    timer.tock("Sorting morton codes");

    timer.tick();
    Node *nodes = new Node[2 * morton_codes.size() - 1];
    auto root_node = generate_hirearchy(nodes, morton_codes.data(), 0, morton_codes.size() - 1, 0);
    timer.tock("Generating hirearchy");

    std::cout << tris.size() << std::endl;
    std::cout << morton_codes.size() << std::endl;
    std::cout << root_node << std::endl;
    return 0;
}