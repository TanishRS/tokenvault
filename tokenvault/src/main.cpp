#include "Registry.h"
#include "RevocationTrail.h"
#include "TransactionLoop.h"
#include "KeyLookup.h"
#include "PoolSorter.h"
#include "LiquidityGraph.h"
#include "SpaceAllocator.h"

#include <iostream>
#include <iomanip>
#include <limits>
#include <string>

// ---------------------------------------------------------------------------
//  Shared system state (lives for the whole session so features interact).
// ---------------------------------------------------------------------------
static Registry        reg;
static RevocationTrail trail;
static TransactionLoop loop;
static KeyLookup        keys;
static PoolSorter      pools;
static LiquidityGraph  graph;
static SpaceAllocator  alloc;

static void line(const std::string& t) {
    std::cout << "\n========== " << t << " ==========\n";
}

// Read a full line of input safely (handles the newline left by >> reads).
static std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

static long long readInt(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string s;
        std::getline(std::cin, s);
        try { return std::stoll(s); }
        catch (...) { std::cout << "  (please enter a number)\n"; }
    }
}

static double readDouble(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string s;
        std::getline(std::cin, s);
        try { return std::stod(s); }
        catch (...) { std::cout << "  (please enter a number)\n"; }
    }
}

// ---------------------------------------------------------------------------
//  Seed some data so the menu isn't empty on first run.
// ---------------------------------------------------------------------------
static void seed() {
    reg.upsert("alice", 1000, "PK_ALICE");
    reg.upsert("bob",    500, "PK_BOB");
    reg.upsert("carol",  750, "PK_CAROL");
    keys.registerKey("PK_ALICE");
    keys.registerKey("PK_BOB");
    keys.registerKey("PK_CAROL");
    pools.setVelocity("poolA", 120);
    pools.setVelocity("poolB", 340);
    pools.setVelocity("poolC", 210);
    graph.addSwap("USDC", "ETH",  0.10, 0.05);
    graph.addSwap("ETH",  "DAI",  0.08, 0.04);
    graph.addSwap("USDC", "DAI",  0.40, 0.30);
    graph.addSwap("DAI",  "WBTC", 0.05, 0.03);
    graph.addSwap("ETH",  "WBTC", 0.50, 0.10);
    alloc.addBlock(1000);
    alloc.addBlock(300);
    alloc.addBlock(600);
}

// ---------------------------------------------------------------------------
//  Interactive feature handlers
// ---------------------------------------------------------------------------
static void doRegistry() {
    line("1. Registration Registry  [unordered_map, O(1) avg]");
    std::cout << "  1) Add/update account\n  2) Look up account\n  3) Show count\n";
    long long c = readInt("  choice: ");
    if (c == 1) {
        std::string w = readLine("  wallet: ");
        long long b   = readInt("  balance: ");
        std::string k = readLine("  public key: ");
        reg.upsert(w, (uint64_t)b, k);
        std::cout << "  -> stored.\n";
    } else if (c == 2) {
        std::string w = readLine("  wallet: ");
        if (auto a = reg.find(w))
            std::cout << "  -> balance " << a->balance << ", key " << a->publicKey << "\n";
        else std::cout << "  -> not found.\n";
    } else {
        std::cout << "  accounts: " << reg.size() << "\n";
    }
}

static void doRevocation() {
    line("2. Revocation Trail  [stack, LIFO undo, O(1)]");
    std::cout << "  1) Grant authority\n  2) Revoke authority\n  3) Undo last\n  4) Check entity\n";
    long long c = readInt("  choice: ");
    if (c == 1)      { auto e = readLine("  entity: "); trail.grant(e);  std::cout << "  -> granted.\n"; }
    else if (c == 2) { auto e = readLine("  entity: "); trail.revoke(e); std::cout << "  -> revoked.\n"; }
    else if (c == 3) { auto r = trail.undoLast(); std::cout << "  -> " << (r.empty() ? "nothing to undo" : r) << "\n"; }
    else             { auto e = readLine("  entity: ");
                       std::cout << "  -> has authority? " << std::boolalpha << trail.hasAuthority(e) << "\n"; }
}

static void doTransactions() {
    line("3. Transaction Loop  [queue, FIFO order, O(1)]");
    std::cout << "  1) Submit a transfer\n  2) Process next\n  3) Process all\n  4) Pending count\n";
    long long c = readInt("  choice: ");
    static uint64_t nextId = 100;
    auto settle = [](const Transaction& tx) {
        bool ok = reg.adjustBalance(tx.from, -(int64_t)tx.amount) &&
                  reg.adjustBalance(tx.to,    (int64_t)tx.amount);
        std::cout << "  settled tx#" << tx.id << " " << tx.from << "->" << tx.to
                  << " (" << tx.amount << ") " << (ok ? "OK" : "FAILED") << "\n";
    };
    if (c == 1) {
        std::string f = readLine("  from: ");
        std::string t = readLine("  to: ");
        long long a   = readInt("  amount: ");
        loop.submit({nextId++, f, t, (uint64_t)a});
        std::cout << "  -> queued.\n";
    } else if (c == 2) {
        if (!loop.processNext(settle)) std::cout << "  -> queue empty.\n";
    } else if (c == 3) {
        if (loop.empty()) std::cout << "  -> queue empty.\n";
        while (loop.processNext(settle)) {}
    } else {
        std::cout << "  pending: " << loop.pending() << "\n";
    }
}

static void doKeys() {
    line("4. Key Lookup  [unordered_set, O(1) avg]");
    std::cout << "  1) Register key\n  2) Revoke key\n  3) Validate key\n";
    long long c = readInt("  choice: ");
    auto k = readLine("  public key: ");
    if (c == 1)      { keys.registerKey(k); std::cout << "  -> registered.\n"; }
    else if (c == 2) { keys.revokeKey(k);   std::cout << "  -> revoked.\n"; }
    else             { std::cout << "  -> valid? " << std::boolalpha << keys.isValid(k) << "\n"; }
}

static void doPools() {
    line("5. Pool Sorter  [priority_queue max-heap]");
    std::cout << "  1) Set/update pool velocity\n  2) Show fastest\n";
    long long c = readInt("  choice: ");
    if (c == 1) {
        auto p = readLine("  pool name: ");
        long long v = readInt("  velocity (tx/s): ");
        pools.setVelocity(p, (uint64_t)v);
        std::cout << "  -> updated.\n";
    } else {
        std::string fp; uint64_t fv;
        if (pools.fastest(fp, fv)) std::cout << "  -> fastest: " << fp << " (" << fv << " tx/s)\n";
        else std::cout << "  -> no pools.\n";
    }
}

static void doGraph() {
    line("6+7. Connection Map & Route Path  [graph + Dijkstra]");
    std::cout << "  1) Add swap edge\n  2) Find lowest-risk route\n  3) Pool count\n";
    long long c = readInt("  choice: ");
    if (c == 1) {
        auto a = readLine("  token A: ");
        auto b = readLine("  token B: ");
        double risk = readDouble("  risk: ");
        double fee  = readDouble("  fee: ");
        graph.addSwap(a, b, risk, fee);
        std::cout << "  -> edge added (cost " << (risk + fee) << ").\n";
    } else if (c == 2) {
        auto s = readLine("  from token: ");
        auto d = readLine("  to token: ");
        auto r = graph.lowestRiskRoute(s, d);
        if (!r.found) { std::cout << "  -> no route (unknown token or unreachable).\n"; return; }
        std::cout << "  -> cost " << std::fixed << std::setprecision(2) << r.totalCost << ": ";
        for (size_t i = 0; i < r.path.size(); ++i)
            std::cout << r.path[i] << (i + 1 < r.path.size() ? " -> " : "\n");
    } else {
        std::cout << "  pools: " << graph.poolCount() << "\n";
    }
}

static void doAlloc() {
    line("8. Space Allocator  [best-fit bin packing]");
    std::cout << "  1) Add a block\n  2) Allocate metadata\n";
    long long c = readInt("  choice: ");
    if (c == 1) {
        long long cap = readInt("  block capacity (bytes): ");
        int idx = alloc.addBlock((uint64_t)cap);
        std::cout << "  -> added as block " << idx << "\n";
    } else {
        long long s = readInt("  size to store (bytes): ");
        auto p = alloc.allocate((uint64_t)s);
        if (p.placed) std::cout << "  -> block " << p.blockIndex
                                << " (best fit, " << p.remainingAfter << " bytes left)\n";
        else std::cout << "  -> NO FIT in any block.\n";
    }
}

// ---------------------------------------------------------------------------
//  Non-interactive auto-demo (original behaviour, kept for quick grading).
// ---------------------------------------------------------------------------
static void autoDemo() {
    line("AUTO DEMO - all 8 features");
    if (auto a = reg.find("alice"))
        std::cout << "Registry: alice balance " << a->balance << "\n";
    trail.grant("minter_Y"); trail.revoke("minter_Y");
    std::cout << "Revocation: " << trail.undoLast() << "\n";
    loop.submit({1,"alice","bob",100});
    loop.processNext([](const Transaction& t){
        std::cout << "Txn: settled tx#" << t.id << " " << t.from << "->" << t.to << "\n"; });
    std::cout << "KeyLookup: PK_ALICE valid? " << std::boolalpha << keys.isValid("PK_ALICE") << "\n";
    std::string fp; uint64_t fv; pools.fastest(fp, fv);
    std::cout << "PoolSorter: fastest " << fp << " (" << fv << ")\n";
    auto r = graph.lowestRiskRoute("USDC","WBTC");
    std::cout << "RoutePath: cost " << r.totalCost << " via " << r.path.size() << " hops\n";
    auto p = alloc.allocate(250);
    std::cout << "Allocator: 250B -> block " << p.blockIndex << "\n";
}

int main() {
    seed();
    std::cout << "TokenVault - Digital Licensing Clearance Token Bank (interactive)\n";
    while (true) {
        std::cout << "\n-----------------------------------------\n"
                  << "  1) Registration Registry\n"
                  << "  2) Revocation Trail\n"
                  << "  3) Transaction Loop\n"
                  << "  4) Key Lookup\n"
                  << "  5) Pool Sorter\n"
                  << "  6) Connection Map & Route Path\n"
                  << "  7) Space Allocator\n"
                  << "  8) Run auto-demo (all features)\n"
                  << "  0) Exit\n"
                  << "-----------------------------------------\n";
        long long c = readInt("Select feature: ");
        switch (c) {
            case 1: doRegistry();      break;
            case 2: doRevocation();    break;
            case 3: doTransactions();  break;
            case 4: doKeys();          break;
            case 5: doPools();         break;
            case 6: doGraph();         break;
            case 7: doAlloc();         break;
            case 8: autoDemo();        break;
            case 0: std::cout << "Goodbye.\n"; return 0;
            default: std::cout << "Invalid choice.\n";
        }
    }
}
