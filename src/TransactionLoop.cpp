#include "TransactionLoop.h"

void TransactionLoop::submit(const Transaction& tx) { q_.push(tx); }

bool   TransactionLoop::empty()   const { return q_.empty(); }
size_t TransactionLoop::pending() const { return q_.size(); }

bool TransactionLoop::processNext(const std::function<void(const Transaction&)>& settle) {
    if (q_.empty()) return false;
    Transaction tx = q_.front();
    q_.pop();
    settle(tx);
    return true;
}
