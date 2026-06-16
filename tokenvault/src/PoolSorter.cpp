#include "PoolSorter.h"

void PoolSorter::setVelocity(const std::string& pool, uint64_t velocity) {
    live_[pool] = velocity;          // authoritative current value
    heap_.push({velocity, pool});    // stale copies tolerated, skipped on pop
}

bool PoolSorter::fastest(std::string& poolOut, uint64_t& velocityOut) {
    while (!heap_.empty()) {
        Entry e = heap_.top();
        auto it = live_.find(e.pool);
        // Skip entries that are stale (velocity no longer matches the live map)
        // or belong to a removed pool.
        if (it == live_.end() || it->second != e.velocity) {
            heap_.pop();
            continue;
        }
        poolOut     = e.pool;
        velocityOut = e.velocity;
        return true;
    }
    return false;
}
