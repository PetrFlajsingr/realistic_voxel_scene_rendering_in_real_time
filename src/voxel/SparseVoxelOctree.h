//
// Created by petr on 12/15/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SPARSEVOXELOCTREE_H
#define REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SPARSEVOXELOCTREE_H

#include <algorithm>

namespace pf::vox {

// tree node describing children of the given node
// must be 64 bytes
struct alignas(64) ChildDescriptor {
  // data describing this nodes children
  struct alignas(32) {
    uint32_t
        childPointer : 15;//< points to memory populated by child nodes - specifically to the first one, since they are consecutive
    // in the case of too much data, the child pointer may not be big enough to reference them, @see far
    uint32_t
        far : 1;//< if far is set, then the child pointer only points to a 32 bit pointer which points to correct child descriptors
    // each bit of these masks corresponds to 1 child
    uint32_t validMask : 8;//< if 1 then the child contains a voxel else it doesn't
    uint32_t leafMask : 8; //< if 1 then the child is a leaf else it is another node
    // if not validMask[x] and not leafMask[x] then the space is empty
    // if validMask[x] and not leafMask[x] then the space is represented by another ChildDescriptor
    // if validMask[x] and leafMask[x] then the space contains a voxel and is not further represented by a ChildDescriptor
  } childData;

  // UNUSED FOR NOW
  struct alignas(32) {
    uint32_t
        contourPointer : 24;//< points to memory populated by contours - specifically to the first one, since they are consecutive
    uint32_t contourMask : 8;//< if 1 then the child has a contour associated else it doesn't
  } contourData;
};

class SparseVoxelOctree {
 public:
 private:
};

}// namespace pf::vox
#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SPARSEVOXELOCTREE_H
