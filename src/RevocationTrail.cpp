#include "RevocationTrail.h"

void RevocationTrail::grant(const std::string& entity) {
    authorities_.insert(entity);
    history_.push({Op::Grant, entity});
}

void RevocationTrail::revoke(const std::string& entity) {
    authorities_.erase(entity);
    history_.push({Op::Revoke, entity});
}

std::string RevocationTrail::undoLast() {
    if (history_.empty()) return "";
    Action a = history_.top();
    history_.pop();
    if (a.op == Op::Grant) {
        authorities_.erase(a.entity);   // reverse a grant
        return "Undid GRANT to " + a.entity + " (authority removed)";
    } else {
        authorities_.insert(a.entity);  // reverse a revoke
        return "Undid REVOKE of " + a.entity + " (authority restored)";
    }
}

bool RevocationTrail::hasAuthority(const std::string& entity) const {
    return authorities_.find(entity) != authorities_.end();
}

std::vector<std::string> RevocationTrail::currentAuthorities() const {
    return {authorities_.begin(), authorities_.end()};
}
