#ifndef POOL_SORTER_H
#define POOL_SORTER_H

// ============================================================================
//  Pool Sorter  -->  max-heap (std::priority_queue)
// ----------------------------------------------------------------------------
//  Arrange mint pools by processing velocity; the highest-velocity pool must
//  be available first for routing during high demand.
//
//  DS CHOICE: max-heap keyed by velocity.
//    - "Give me the current fastest pool" = repeated max-extraction.
//      top() O(1), push/pop O(log n).
//    - Fully sorting the list each query is O(n log n) every time; a heap
//      pays O(log n) per update and O(1) to peek the winner.
//    - Velocities change as load shifts, so we re-push updated snapshots and
//      lazily skip stale entries on pop (standard heap-with-updates trick).
//  REAL WORLD: routers/relayers pick the highest-throughput venue first;
//  priority scheduling is a heap.
// ============================================================================

#include <queue>
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>

class PoolSorter {
public:
    // Insert a pool or update its velocity. O(log n).
    void setVelocity(const std::string& pool, uint64_t velocity);

    // Peek the highest-velocity pool without removing it.
    // Returns false if no live pools. O(log n) worst (discards stale tops).
    bool fastest(std::string& poolOut, uint64_t& velocityOut);

private:
    struct Entry {
        uint64_t    velocity;
        std::string pool;
        bool operator<(const Entry& o) const { return velocity < o.velocity; } // max-heap
    };
    std::priority_queue<Entry>                  heap_;
    std::unordered_map<std::string, uint64_t>   live_;   // pool -> current velocity
};

#endif // POOL_SORTER_H
