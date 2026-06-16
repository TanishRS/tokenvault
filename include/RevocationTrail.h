#ifndef REVOCATION_TRAIL_H
#define REVOCATION_TRAIL_H

// ============================================================================
//  Revocation Trail  -->  stack (std::stack)
// ----------------------------------------------------------------------------
//  Every grant / revoke of a minting authority is recorded. A mistaken
//  revoke must "instantly reverse the last action and restore previous
//  permissions" -> classic LIFO undo.
//
//  DS CHOICE: stack.
//    - The newest action is always the one to undo -> top() / pop() = O(1).
//    - A queue (FIFO) would undo the OLDEST action first, which is wrong.
//    - An array works but a stack states the intent (LIFO) in the type itself.
//  REAL WORLD: on-chain admin actions are append-only logs; reversing the
//  most recent privileged action before block finalization is the LIFO case.
// ============================================================================

#include <stack>
#include <string>
#include <unordered_set>
#include <vector>

class RevocationTrail {
public:
    // Grant authority to an entity; pushes an undoable record. O(1).
    void grant(const std::string& entity);

    // Revoke authority from an entity; pushes an undoable record. O(1).
    void revoke(const std::string& entity);

    // Reverse the most recent grant/revoke. Returns a human-readable
    // description of what was undone, or "" if nothing to undo. O(1).
    std::string undoLast();

    bool hasAuthority(const std::string& entity) const;
    std::vector<std::string> currentAuthorities() const;

private:
    enum class Op { Grant, Revoke };
    struct Action { Op op; std::string entity; };

    std::stack<Action>              history_;
    std::unordered_set<std::string> authorities_;  // current live set
};

#endif // REVOCATION_TRAIL_H
