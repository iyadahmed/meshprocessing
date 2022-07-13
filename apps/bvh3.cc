#include <cmath>
#include <cstdint>

#ifdef _MSC_VER
#include <intrin.h>
#include <Windows.h>
uint32_t __inline ctz( uint32_t value )
{
    DWORD trailing_zero = 0;

    if ( _BitScanForward( &trailing_zero, value ) )
    {
        return trailing_zero;
    }
    else
    {
        // This is undefined, I better choose 32 than 0
        return 32;
    }
}

uint32_t __inline clz( uint32_t value )
{
    DWORD leading_zero = 0;

    if ( _BitScanReverse( &leading_zero, value ) )
    {
       return 31 - leading_zero;
    }
    else
    {
         // Same remarks as above
         return 32;
    }
}
#endif


// https://developer.nvidia.com/blog/thinking-parallel-part-iii-tree-construction-gpu/

// Expands a 10-bit integer into 30 bits
// by inserting 2 zeros after each bit.
unsigned int expand_bits(unsigned int v)
{
    v = (v * 0x00010001u) & 0xFF0000FFu;
    v = (v * 0x00000101u) & 0x0F00F00Fu;
    v = (v * 0x00000011u) & 0xC30C30C3u;
    v = (v * 0x00000005u) & 0x49249249u;
    return v;
}

// Calculates a 30-bit Morton code for the
// given 3D point located within the unit cube [0,1].
unsigned int morton3D(float x, float y, float z)
{
    x = fminf(fmaxf(x * 1024.0f, 0.0f), 1023.0f);
    y = fminf(fmaxf(y * 1024.0f, 0.0f), 1023.0f);
    z = fminf(fmaxf(z * 1024.0f, 0.0f), 1023.0f);
    unsigned int xx = expand_bits((unsigned int)x);
    unsigned int yy = expand_bits((unsigned int)y);
    unsigned int zz = expand_bits((unsigned int)z);
    return xx * 4 + yy * 2 + zz;
}

int find_split(unsigned int *sorted_morton_codes,
              int first,
              int last)
{
    // Identical Morton codes => split the range in the middle.

    unsigned int first_code = sorted_morton_codes[first];
    unsigned int last_code = sorted_morton_codes[last];

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
            unsigned int splitCode = sorted_morton_codes[new_split];
            int split_prefix = clz(first_code ^ splitCode);
            if (split_prefix > common_prefix)
                split = new_split; // accept proposal
        }
    } while (step > 1);

    return split;
}

struct Node
{
    Node *child_a;
    Node *child_b;
    int primitive_id;
};

#define LeafNode(id) Node{nullptr, nullptr, id};
#define InternalNode(child_a, child_b) Node{child_a, child_b, -1};

Node *generate_hirearchy(unsigned int *sorted_morton_codes,
                         int *sorted_primitive_id,
                         int first,
                         int last)
{
    // Single object => create a leaf node.

    if (first == last)
        return new LeafNode(sorted_primitive_id[first]);

    // Determine where to split the range.

    int split = find_split(sorted_morton_codes, first, last);

    // Process the resulting sub-ranges recursively.

    Node *child_a = generate_hirearchy(sorted_morton_codes, sorted_primitive_id,
                                      first, split);
    Node *child_b = generate_hirearchy(sorted_morton_codes, sorted_primitive_id,
                                      split + 1, last);
    return new InternalNode(child_a, child_b);
}

int main(int argc, char** argv)
{
    return 0;
}