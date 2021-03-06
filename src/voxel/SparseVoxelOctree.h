/**
 * @file SparseVoxelOctree.h
 * @brief Structs for sparse voxel octree.
 * @author Petr Flajšingr
 * @date 15.12.20
 */

#ifndef REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SPARSEVOXELOCTREE_H
#define REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SPARSEVOXELOCTREE_H

#include <algorithm>
#include <span>

namespace pf::vox {

constexpr uint32_t PAGE_SIZE = 8192;
/* this is the ESVO paper correct version
// tree node describing children of the given node
// must be 64 bytes
struct alignas(8) ChildDescriptor {
  // data describing this nodes children
  struct alignas(4) {
    // each bit of these masks corresponds to 1 child
    uint32_t leafMask : 8; //< if 1 then the child is a leaf else it is another node
    uint32_t validMask : 8;//< if 1 then the child contains a voxel else it doesn't
    // if not validMask[x] and not leafMask[x] then the space is empty
    // if validMask[x] and not leafMask[x] then the space is represented by another ChildDescriptor
    // if validMask[x] and leafMask[x] then the space contains a voxel and is not further represented by a ChildDescriptor
    uint32_t
        far : 1;//< if far is set, then the child pointer only points to a 32 bit pointer which points to correct child descriptors
    uint32_t
        childPointer : 15;//< points to memory populated by child nodes - specifically to the first one, since they are consecutive
    // in the case of too much data, the child pointer may not be big enough to reference them, @see far
  } childData;

  struct alignas(4) {
    uint32_t UNUSED;
  } shadingData;

  [[nodiscard]] std::string toString() const;
  [[nodiscard]] std::string stringDraw() const;
};*/

// tree node describing children of the given node
// must be 64 bytes
struct alignas(8) ChildDescriptor {

  // each bit of these masks corresponds to 1 child
  uint8_t leafMask; //< if 1 then the child is a leaf else it is another node
  uint8_t validMask;//< if 1 then the child contains a voxel else it doesn't
  // if not validMask[x] and not leafMask[x] then the space is empty
  // if validMask[x] and not leafMask[x] then the space is represented by another ChildDescriptor
  // if validMask[x] and leafMask[x] then the space contains a voxel and is not further represented by a ChildDescriptor

  uint16_t UNUSED;

  uint32_t
      childPointer;//< points to memory populated by child nodes - specifically to the first one, since they are consecutive
  [[nodiscard]] std::string toString() const;
  [[nodiscard]] std::string stringDraw() const;
};

struct alignas(4) Contour {
  uint32_t nz : 6;
  uint32_t ny : 6;
  uint32_t nx : 6;
  uint32_t position : 7;
  uint32_t thickness : 7;
};

struct alignas(4) AttachmentLookupEntry {
  uint32_t mask : 8;         //< if 1 then the voxel has an attribute entry else it doesn't
  uint32_t valuePointer : 24;//< points to an attachment in consecutive buffer
};
/*
struct alignas(4) PhongAttachment {
  struct alignas(4) {
    uint8_t alpha;
    uint8_t blue;
    uint8_t green;
    uint8_t red;
  } color;
};*/
struct alignas(4) MaterialIndexAttachment {
  std::uint32_t materialId;
};

struct alignas(8) PageHeader {
  uint32_t infoSectionPointer;
  uint32_t attachmentsPointer;
};

struct Page {
  PageHeader header;
  std::vector<ChildDescriptor> childDescriptors;
  std::vector<uint32_t> farPointers;

  [[nodiscard]] std::vector<std::byte> serialize() const;
  [[nodiscard]] static Page Deserialize(std::span<const std::byte> data);

  [[nodiscard]] std::string toString() const;
};

struct ContourData {
  std::vector<Contour> contours;
};

struct Attachments {
  std::vector<AttachmentLookupEntry> lookupEntries;
  std::vector<MaterialIndexAttachment> attachments;
};

struct InfoSection {
  Attachments attachments;
};

struct Block {
  std::vector<Page> pages;
  InfoSection infoSection;

  [[nodiscard]] std::vector<std::byte> serialize() const;
  [[nodiscard]] static Block Deserialize(std::span<const std::byte> data);
};

class SparseVoxelOctree {
 public:
  explicit SparseVoxelOctree(std::vector<Block> &&blocks = {});

  [[nodiscard]] std::vector<std::byte> serialize() const;

  [[nodiscard]] static SparseVoxelOctree Deserialize(std::span<const std::byte> data);

  void addBlock(Block &&block);

  [[nodiscard]] const std::vector<Block> &getBlocks() const;

 private:
  std::vector<Block> blocks;
};

}// namespace pf::vox
#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SPARSEVOXELOCTREE_H
