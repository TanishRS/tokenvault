#ifndef REGISTRY_H
#define REGISTRY_H

// ============================================================================
//  Registration Registry  -->  hash table (std::unordered_map)
// ----------------------------------------------------------------------------
//  Stores a token-balance account for every wallet. Must stay fast (create /
//  update / lookup) even at millions of wallets.
//
//  DS CHOICE: hash table keyed by wallet address.
//    - O(1) average insert / lookup / update -> stays flat as user count grows.
//    - A sorted array would be O(log n) lookup but O(n) insert (shifting);
//      a balanced BST would be O(log n) everything. Neither matches the "fast
//      even at millions" requirement the way O(1) average does.
//  REAL WORLD: Coinbase / any custodial exchange keeps an account-balance
//  table keyed by user id — same access pattern.
// ============================================================================

#include <string>
#include <unordered_map>
#include <cstdint>

struct Account {
    std::string wallet;
    uint64_t    balance = 0;   // token units
    std::string publicKey;     // links to KeyLookup
};

class Registry {
public:
    // Create or overwrite an account. O(1) average.
    void upsert(const std::string& wallet, uint64_t balance, const std::string& publicKey);

    // Adjust balance by a signed delta. Returns false if account missing
    // or the debit would underflow. O(1) average.
    bool adjustBalance(const std::string& wallet, int64_t delta);

    // Look up an account. Returns nullptr if absent. O(1) average.
    const Account* find(const std::string& wallet) const;

    bool   exists(const std::string& wallet) const;
    size_t size() const { return table_.size(); }

private:
    std::unordered_map<std::string, Account> table_;
};

#endif // REGISTRY_H
