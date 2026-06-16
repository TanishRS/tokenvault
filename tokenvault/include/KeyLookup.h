#ifndef KEY_LOOKUP_H
#define KEY_LOOKUP_H

// ============================================================================
//  Key Lookup  -->  hash set (std::unordered_set)
// ----------------------------------------------------------------------------
//  Verify a public key is valid/registered using just the key, "without
//  scanning the entire key registry."
//
//  DS CHOICE: hash set.
//    - O(1) average membership test -> the "no full scan" requirement is
//      literally the difference between O(1) hashing and O(n) linear search.
//    - A sorted vector + binary search is O(log n); fine, but O(1) is the
//      stronger answer and matches the wording.
//  REAL WORLD: validators check a key/nonce against a set membership before
//  admitting a transaction.
// ============================================================================

#include <string>
#include <unordered_set>

class KeyLookup {
public:
    void registerKey(const std::string& pubKey);   // O(1) avg
    void revokeKey(const std::string& pubKey);      // O(1) avg
    bool isValid(const std::string& pubKey) const;  // O(1) avg
    size_t size() const { return keys_.size(); }
private:
    std::unordered_set<std::string> keys_;
};

#endif // KEY_LOOKUP_H
