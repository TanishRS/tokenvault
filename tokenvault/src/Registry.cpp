#include "Registry.h"

void Registry::upsert(const std::string& wallet, uint64_t balance, const std::string& publicKey) {
    Account& a = table_[wallet];   // creates if absent
    a.wallet    = wallet;
    a.balance   = balance;
    a.publicKey = publicKey;
}

bool Registry::adjustBalance(const std::string& wallet, int64_t delta) {
    auto it = table_.find(wallet);
    if (it == table_.end()) return false;
    if (delta < 0 && static_cast<uint64_t>(-delta) > it->second.balance)
        return false;                      // would underflow
    it->second.balance = static_cast<uint64_t>(
        static_cast<int64_t>(it->second.balance) + delta);
    return true;
}

const Account* Registry::find(const std::string& wallet) const {
    auto it = table_.find(wallet);
    return it == table_.end() ? nullptr : &it->second;
}

bool Registry::exists(const std::string& wallet) const {
    return table_.find(wallet) != table_.end();
}
