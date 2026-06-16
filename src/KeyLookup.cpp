#include "KeyLookup.h"

void KeyLookup::registerKey(const std::string& pubKey) { keys_.insert(pubKey); }
void KeyLookup::revokeKey(const std::string& pubKey)   { keys_.erase(pubKey);  }
bool KeyLookup::isValid(const std::string& pubKey) const {
    return keys_.find(pubKey) != keys_.end();
}
