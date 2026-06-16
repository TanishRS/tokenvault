#include "SpaceAllocator.h"
#include <limits>

int SpaceAllocator::addBlock(uint64_t capacity) {
    blocks_.push_back({capacity, 0});
    return static_cast<int>(blocks_.size()) - 1;
}

SpaceAllocator::Placement SpaceAllocator::allocate(uint64_t size) {
    Placement p;
    uint64_t bestLeftover = std::numeric_limits<uint64_t>::max();
    int bestIdx = -1;

    for (int i = 0; i < static_cast<int>(blocks_.size()); ++i) {
        uint64_t freeBytes = blocks_[i].capacity - blocks_[i].used;
        if (freeBytes >= size) {                 // it fits
            uint64_t leftover = freeBytes - size;
            if (leftover < bestLeftover) {        // tightest fit so far
                bestLeftover = leftover;
                bestIdx = i;
            }
        }
    }

    if (bestIdx == -1) return p;                  // no block can hold it
    blocks_[bestIdx].used += size;
    p.placed = true;
    p.blockIndex = bestIdx;
    p.remainingAfter = blocks_[bestIdx].capacity - blocks_[bestIdx].used;
    return p;
}

uint64_t SpaceAllocator::freeSpace(int blockIndex) const {
    const Block& b = blocks_[blockIndex];
    return b.capacity - b.used;
}
