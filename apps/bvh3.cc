#include <cmath>

// #include <CL/sycl.hpp>

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
    x = std::min(std::max(x * 1024.0f, 0.0f), 1023.0f);
    y = std::min(std::max(y * 1024.0f, 0.0f), 1023.0f);
    z = std::min(std::max(z * 1024.0f, 0.0f), 1023.0f);
    unsigned int xx = expand_bits((unsigned int)x);
    unsigned int yy = expand_bits((unsigned int)y);
    unsigned int zz = expand_bits((unsigned int)z);
    return xx * 4 + yy * 2 + zz;
}

int findSplit(unsigned int *sortedMortonCodes,
              int first,
              int last)
{
    // Identical Morton codes => split the range in the middle.

    unsigned int firstCode = sortedMortonCodes[first];
    unsigned int lastCode = sortedMortonCodes[last];

    if (firstCode == lastCode)
        return (first + last) >> 1;

    // Calculate the number of highest bits that are the same
    // for all objects, using the count-leading-zeros intrinsic.

    int commonPrefix = __builtin_clz(firstCode ^ lastCode);

    // Use binary search to find where the next bit differs.
    // Specifically, we are looking for the highest object that
    // shares more than commonPrefix bits with the first one.

    int split = first; // initial guess
    int step = last - first;

    do
    {
        step = (step + 1) >> 1;      // exponential decrease
        int newSplit = split + step; // proposed new position

        if (newSplit < last)
        {
            unsigned int splitCode = sortedMortonCodes[newSplit];
            int splitPrefix = __builtin_clz(firstCode ^ splitCode);
            if (splitPrefix > commonPrefix)
                split = newSplit; // accept proposal
        }
    } while (step > 1);

    return split;
}

struct Node
{
    Node *parent;
    Node *childA;
    Node *childB;
    int objectID;
};

// Node *generate_hirearchy(unsigned int *sortedMortonCodes,
//                          int *sortedObjectIDs,
//                          int first,
//                          int last)
// {
//     // Single object => create a leaf node.

//     if (first == last)
//         return new LeafNode(&sortedObjectIDs[first]);

//     // Determine where to split the range.

//     int split = findSplit(sortedMortonCodes, first, last);

//     // Process the resulting sub-ranges recursively.

//     Node *childA = generate_hirearchy(sortedMortonCodes, sortedObjectIDs,
//                                       first, split);
//     Node *childB = generate_hirearchy(sortedMortonCodes, sortedObjectIDs,
//                                       split + 1, last);
//     return new InternalNode(childA, childB);
// }

/*
Copyright (c) 2015-2016 Advanced Micro Devices, Inc. All rights reserved.
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <hip/hip_runtime.h>
#include <hip/hip_runtime_api.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>

#define SAMPLE_VERSION "HIP-Examples-Application-v1.0"
#define SUCCESS 0
#define FAILURE 1

using namespace std;

__global__ void helloworld(char *in, char *out)
{
    int num = hipThreadIdx_x + hipBlockDim_x * hipBlockIdx_x;
    out[num] = in[num] + 1;
}

int main(int argc, char *argv[])
{
    hipDeviceProp_t devProp;
    hipGetDeviceProperties(&devProp, 0);
    cout << " System minor " << devProp.minor << endl;
    cout << " System major " << devProp.major << endl;
    cout << " agent prop name " << devProp.name << endl;

    /* Initial input,output for the host and create memory objects for the kernel*/
    const char *input = "GdkknVnqkc";
    size_t strlength = strlen(input);
    cout << "input string:" << endl;
    cout << input << endl;
    char *output = (char *)malloc(strlength + 1);

    char *inputBuffer;
    char *outputBuffer;
    hipMalloc((void **)&inputBuffer, (strlength + 1) * sizeof(char));
    hipMalloc((void **)&outputBuffer, (strlength + 1) * sizeof(char));

    hipMemcpy(inputBuffer, input, (strlength + 1) * sizeof(char), hipMemcpyHostToDevice);

    hipLaunchKernelGGL(helloworld,
                       dim3(1),
                       dim3(strlength),
                       0, 0,
                       inputBuffer, outputBuffer);

    hipMemcpy(output, outputBuffer, (strlength + 1) * sizeof(char), hipMemcpyDeviceToHost);

    hipFree(inputBuffer);
    hipFree(outputBuffer);

    output[strlength] = '\0'; // Add the terminal character to the end of output.
    cout << "\noutput string:" << endl;
    cout << output << endl;

    free(output);

    std::cout << "Passed!\n";
    return SUCCESS;
}
