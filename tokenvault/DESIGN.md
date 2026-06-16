# TokenVault — Design & Data-Structure Justification

A C++17 implementation of the **Digital Licensing Clearance Token Bank**.
Each "Must Have Feature" maps to one data structure chosen for the access
pattern it describes. STL is used where it *is* the correct, idiomatic tool;
the graph + pathfinding and the allocator are hand-built because that logic is
the substance of the problem.

## Feature → Data Structure → Why

| # | Feature | Data Structure | Key Ops & Complexity | Why this one (vs. alternatives) |
|---|---------|----------------|----------------------|----------------------------------|
| 1 | Registration Registry | Hash table (`unordered_map`) | insert / lookup / update **O(1) avg** | "Fast even at millions of wallets" = constant-time keyed access. Sorted array is O(n) insert; BST is O(log n) everything. |
| 2 | Revocation Trail | Stack (`stack`) | push / undo **O(1)** | "Instantly reverse the *last* action" = LIFO. A queue would undo the oldest action first — wrong. |
| 3 | Transaction Loop | FIFO queue (`queue`) | enqueue / dequeue **O(1)** | "Process in the exact order submitted" = FIFO. A stack reverses order; a priority queue reorders — both break consistency. |
| 4 | Key Lookup | Hash set (`unordered_set`) | membership **O(1) avg** | "Verify without scanning the entire registry" is literally O(1) hashing vs O(n) scan. |
| 5 | Pool Sorter | Max-heap (`priority_queue`) | peek max **O(1)**, update **O(log n)** | "Highest-velocity pool first" = repeated max-extraction. Re-sorting the list is O(n log n) per query. |
| 6 | Connection Map | Graph (adjacency list, custom) | space **O(V+E)** | Liquidity graph is sparse: pools connect to a few peers, not all. Adjacency matrix wastes O(V²). |
| 7 | Route Path | Dijkstra + min-heap (custom) | **O((V+E) log V)** | Lowest-cost path with non-negative weights (risk+fee). BFS gives fewest hops not lowest cost; Bellman-Ford is slower and only needed for negative edges. |
| 8 | Space Allocator | Best-fit bin packing (custom) | scan **O(n)** (note: O(log n) via balanced tree of free sizes) | "Best space utilization / minimize waste" = Best-Fit heuristic. First-Fit wastes more; Worst-Fit maximizes leftover (opposite goal). |

## Real-world mapping

- **Registry / Key Lookup** — custodial exchanges (Coinbase) keep balance and
  key/nonce tables keyed by user id for O(1) admission checks.
- **Transaction Loop** — matching/settlement engines process orders in
  time-priority FIFO so balances stay consistent.
- **Pool Sorter** — routers pick the highest-throughput venue first under load;
  priority scheduling is a heap.
- **Connection Map + Route Path** — this is exactly how a DEX aggregator
  (1inch, Uniswap auto-routing) finds the best multi-hop swap across pools.
- **Space Allocator** — block builders pack metadata/transactions to maximize
  fee-per-byte and minimize unused block space.

## Build & run

```
make run        # compiles all modules and runs the demo
make clean       # removes objects and binary
```

Requires `g++` with C++17. Builds clean under `-Wall -Wextra`.

## Layout

```
tokenvault/
├── include/        # one header per feature, with the DS justification on top
├── src/            # matching implementations + main.cpp driver
├── Makefile
└── DESIGN.md
```

`main.cpp` exercises all 8 features end-to-end. Notable demo proofs:
- Revocation Trail restores a mistakenly-revoked authority via `undoLast()`.
- Route Path picks USDC→ETH→DAI→WBTC (cost 0.35) over the direct but pricier
  USDC→DAI edge — proving multi-hop optimization, not just direct lookup.
- Space Allocator places 250 bytes into the 300-byte block (tightest fit),
  not the empty 1000-byte block.
