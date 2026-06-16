#ifndef TRANSACTION_LOOP_H
#define TRANSACTION_LOOP_H

// ============================================================================
//  Transaction Loop  -->  FIFO queue (std::queue)
// ----------------------------------------------------------------------------
//  Transfers/settlements arrive continuously and MUST be processed in the
//  exact order submitted so balances stay consistent.
//
//  DS CHOICE: queue.
//    - FIFO is exactly "first submitted, first settled" -> push back O(1),
//      pop front O(1).
//    - A stack would settle newest-first (LIFO) -> wrong order, races.
//    - A priority_queue would reorder by some key -> also violates submission
//      order. (Note: Pool Sorter DOES want a heap; this one explicitly does
//      not — that contrast is the point of the question.)
//  REAL WORLD: exchange matching/settlement engines process orders in
//  time-priority FIFO within a price level.
// ============================================================================

#include <queue>
#include <string>
#include <cstdint>
#include <functional>

struct Transaction {
    uint64_t    id;
    std::string from;
    std::string to;
    uint64_t    amount;
};

class TransactionLoop {
public:
    void submit(const Transaction& tx);    // enqueue, O(1)
    bool empty() const;
    size_t pending() const;

    // Pop and hand the next tx (in submission order) to a settle callback.
    // Returns false if the queue was empty. O(1).
    bool processNext(const std::function<void(const Transaction&)>& settle);

private:
    std::queue<Transaction> q_;
};

#endif // TRANSACTION_LOOP_H
