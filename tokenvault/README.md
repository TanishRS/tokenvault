# TokenVault — Digital Licensing Clearance Token Bank

> DSA-I Using C++ — Project VIVA Submission
> Tanish Ramesh Suvarna · 150096725082 · B.Tech CSE (Cohort: Larry Page) · Sem 2
> ITM Skills University · Instructor: Aarti Shalindrasingh Pardeshi · 16 June 2026

---

## 2.1 Project Title

**TokenVault — Digital Licensing Clearance Token Bank**

A console-based C++17 system that manages a digital token platform end to end,
demonstrating eight core Data Structures and Algorithms — each chosen to match
a specific real-world access pattern and justified accordingly.

## 2.2 Problem Statement

TokenVault is a digital asset platform that manages token balances for millions
of users, verifies cryptographic keys for every transaction, and facilitates
trades across decentralized exchanges. In its current state the platform has
several problems:

- Allocating and tracking token balance accounts is slow as the platform scales.
- When a minting authority is revoked by mistake, there is no way to trace or
  reverse it.
- Token transfer requests pile up randomly instead of being processed in
  submission order.
- Validating a user's cryptographic public key requires scanning the entire
  registry.
- There is no map of how decentralized-exchange liquidity pools connect, so
  finding the safest conversion route through intermediate swaps is impossible.
- Blockchain storage is wasted because there is no intelligent logic for packing
  metadata into blocks.

The system must provide fast token balance account management, reliable
authority-revocation rollback, strict in-order transaction processing, instant
public-key validation, ranking of mint pools by processing speed, a
liquidity-pool connection map, the lowest-risk multi-step conversion route, and
smart block-storage allocation that maximizes space utilization.

## 2.3 Objectives

1. Manage token-balance accounts with constant-time create / update / lookup.
2. Maintain an undoable trail of authority grants and revokes for instant rollback.
3. Process transactions strictly in submission order to keep balances consistent.
4. Validate public keys instantly without scanning the full registry.
5. Rank mint pools by velocity and surface the fastest for routing decisions.
6. Model liquidity pools and swap pairs as a graph.
7. Compute the lowest-risk/fee conversion route across multiple hops.
8. Allocate block storage using a best-fit strategy to minimize wasted space.
9. For each feature, justify the chosen data structure against alternatives and
   relate it to a real platform such as Uniswap or Coinbase.

## 2.4 System Overview / Architecture

The system is a single console application composed of eight independent modules,
one per feature. Each module is a self-contained class with its own header
(interface + justification comment) and implementation file. A central driver
(`main.cpp`) wires them together, holds the shared system state, and exposes both
an interactive menu and a one-shot auto-demo.

```
                    +------------------------+
                    |        main.cpp        |
                    |  interactive menu /    |
                    |     auto-demo driver   |
                    +-----------+------------+
                                |
   +--------+--------+--------+--+-----+--------+--------+--------+
   |        |        |        |        |        |        |        |
Registry  Revoc.   Txn      Key      Pool    Liquidity         Space
(hash    Trail    Loop     Lookup   Sorter   Graph +           Allocator
 map)    (stack)  (queue)  (hash    (heap)   Dijkstra          (best-fit)
                            set)              (graph)
```

Modules are decoupled: each can be tested in isolation, and the driver is the
only place they interact (e.g. the Transaction Loop calls into the Registry to
settle balances).

## 2.5 Data Structures and Algorithms Used

| # | Feature | Data Structure / Algorithm | Implementation |
|---|---------|----------------------------|----------------|
| 1 | Registration Registry | Hash table | `std::unordered_map` |
| 2 | Revocation Trail | Stack (LIFO) | `std::stack` |
| 3 | Transaction Loop | Queue (FIFO) | `std::queue` |
| 4 | Key Lookup | Hash set | `std::unordered_set` |
| 5 | Pool Sorter | Max-heap (priority queue) | `std::priority_queue` |
| 6 | Connection Map | Graph (adjacency list) | Custom (`LiquidityGraph`) |
| 7 | Route Path | Dijkstra's shortest path + min-heap | Custom (on `LiquidityGraph`) |
| 8 | Space Allocator | Best-fit bin packing | Custom (`SpaceAllocator`) |

**Why each was chosen (justification):**

- **Registration Registry → hash table.** Lookup, insert and update must stay
  fast even at millions of wallets. A hash table gives **O(1)** average for all
  three. A sorted array gives O(n) insertion (shifting); a balanced BST gives
  O(log n) for everything. Constant-time keyed access matches "fast even as the
  platform scales." *Real world: Coinbase keeps an account-balance table keyed by
  user id.*

- **Revocation Trail → stack.** A mistaken revoke must reverse the **most recent**
  action — that is last-in-first-out. `push`/`undo` are **O(1)**. A queue would
  undo the oldest action first, which is incorrect. *Real world: reversing the
  most recent privileged admin action before block finalization.*

- **Transaction Loop → queue.** Transfers must settle in the **exact order
  submitted**. FIFO is precisely this; enqueue and dequeue are **O(1)**. A stack
  reverses order; a priority queue reorders — both break balance consistency.
  *Real world: matching/settlement engines process orders in time priority.*

- **Key Lookup → hash set.** "Verify without scanning the entire registry" is the
  literal difference between **O(1)** hashed membership and O(n) linear scan.
  *Real world: validators check a key/nonce against set membership.*

- **Pool Sorter → max-heap.** "Highest-velocity pool first" is repeated
  maximum-extraction. Peek is **O(1)**, updates are **O(log n)**. Re-sorting the
  whole list on each query would be O(n log n). Velocities change with load, so
  updated snapshots are pushed and stale tops skipped lazily on pop. *Real world:
  routers pick the highest-throughput venue under load.*

- **Connection Map → graph (adjacency list).** Pools are nodes, swap pairs are
  weighted edges (weight = risk + fee). The liquidity graph is **sparse** (each
  pool connects to a handful of others, not all millions), so an adjacency list
  costs **O(V + E)** space versus O(V²) for an adjacency matrix.

- **Route Path → Dijkstra with a min-heap.** Lowest-cost path with non-negative
  weights (risk + fee). Complexity **O((V + E) log V)**. BFS would minimise hop
  count, not cost; Bellman-Ford handles negative edges we do not have and is
  slower. *Real world: this is exactly how a DEX aggregator (1inch / Uniswap
  auto-routing) finds the best multi-hop swap.*

- **Space Allocator → best-fit bin packing.** "Best space utilization / minimize
  waste" is the textbook Best-Fit heuristic: place the item in the block whose
  remaining capacity is the smallest that still fits. First-Fit is faster but
  wastes more; Worst-Fit maximizes leftover (the opposite goal). The simple scan
  is **O(n)** per placement; a balanced tree of free capacities would reduce this
  to O(log n). *Real world: block builders pack metadata to minimize unused space.*

## 2.6 Implementation Approach

- Language: **C++17**, console only (no GUI, web, or database — per submission
  guidelines), compiled with `g++ -std=c++17 -Wall -Wextra -O2`.
- One class per feature, each in its own `.h`/`.cpp` pair under `include/` and
  `src/`. Every header opens with a comment block stating the data-structure
  choice, its complexity, and why alternatives were rejected.
- The Standard Library is used where it **is** the correct, idiomatic tool
  (features 1–5). The graph + Dijkstra and the best-fit allocator are
  **hand-built** (features 6–8) because that algorithmic logic is the substance
  of the exercise.
- `main.cpp` seeds sample data, then offers an interactive menu (select a
  feature, feed your own inputs) plus an auto-demo that runs all eight features
  in sequence.
- Build is automated with a `Makefile` (`make run`).

## 2.7 Time and Space Complexity Analysis

| Feature | Operation | Time | Space |
|---------|-----------|------|-------|
| Registration Registry | insert / find / update | O(1) avg | O(n) accounts |
| Revocation Trail | grant / revoke / undo | O(1) | O(a) actions |
| Transaction Loop | submit / process next | O(1) | O(t) pending |
| Key Lookup | register / validate | O(1) avg | O(k) keys |
| Pool Sorter | update velocity | O(log p) | O(p) pools |
| Pool Sorter | peek fastest | O(1) amortized* | — |
| Connection Map | add pool / add swap | O(1) amortized | O(V + E) |
| Route Path | lowest-risk route | O((V + E) log V) | O(V) |
| Space Allocator | allocate (best-fit) | O(b) blocks | O(b) |

\*Peek may discard stale heap entries; each entry is discarded at most once, so
the cost is amortized O(1) across the run.

## 2.8 Execution Steps

Requires `g++` with C++17 support (and optionally `make`).

**Using the Makefile (recommended):**
```bash
cd tokenvault
make run        # compiles all modules and runs the program
make clean      # removes build artifacts
```

**Manual compilation (if `make` is unavailable):**
```bash
cd tokenvault
g++ -std=c++17 -Iinclude src/*.cpp -o tokenvault
./tokenvault    # on Windows: tokenvault.exe
```

On launch you get a menu. Choose a feature number to interact with it, choose
**8** to run the full auto-demo, or **0** to exit.

## 2.9 Sample Inputs and Outputs

A captured run of the auto-demo (menu option 8) is included in
[`samples/sample_output.txt`](samples/sample_output.txt). Key results:

```
1. Registration Registry : alice balance 1000, key PK_ALICE
2. Revocation Trail       : Undid REVOKE of minter_Y (authority restored)
3. Transaction Loop       : tx#1, tx#2, tx#3 settled in submission order
4. Key Lookup             : PK_ALICE valid? true | PK_FAKE valid? false
5. Pool Sorter            : fastest poolB (340) -> after update, poolA (999)
6+7. Route Path           : USDC -> ETH -> DAI -> WBTC (cost 0.35)
8. Space Allocator        : 250B -> block 1 (best fit, 50 bytes left)
```

Note the two non-trivial results: the router chooses the **3-hop**
USDC→ETH→DAI→WBTC path (cost 0.35) over the direct but pricier USDC→DAI edge,
proving multi-hop optimisation; and the allocator places 250 bytes into the
300-byte block (tightest fit) rather than the empty 1000-byte block.

## 2.10 Screenshots

Screenshots of the program executing are in the [`screenshots/`](screenshots/)
folder:

- `01_menu.png` — the interactive main menu on launch
- `02_auto_demo.png` — full auto-demo output (all 8 features)
- `03_revocation_undo.png` — grant → revoke → undo restoring authority
- `04_route_path.png` — lowest-risk route query

*(Capture these on your machine after running — see the GitHub guide.)*

## 2.11 Results and Observations

- All eight features compile cleanly under `-Wall -Wextra` (zero warnings) and
  run correctly.
- The data-structure choices behave as predicted: the stack correctly performs
  LIFO undo, the queue preserves submission order, the heap surfaces the fastest
  pool after live updates, and Dijkstra finds a cheaper multi-hop route than the
  direct edge.
- The best-fit allocator measurably reduces wasted space versus a naive
  first-available placement, by preferring the tightest-fitting block.
- The clearest insight: matching each access pattern to the right structure is
  what keeps the system fast — O(1) hashing instead of O(n) scans, O(log n) heap
  updates instead of O(n log n) re-sorts.

## 2.12 Conclusion

TokenVault demonstrates that the eight stated platform problems each map cleanly
to a classical data structure or algorithm, and that selecting the right one for
the access pattern — and justifying it against the alternatives — is the core
skill the project tests. The Standard Library handles the standard structures
idiomatically, while the graph, Dijkstra routing, and best-fit allocator are
implemented from scratch to show algorithmic understanding. The result is a
modular, readable, fully executable C++17 system that mirrors how real platforms
such as Uniswap and Coinbase organise balances, ordering, routing, and storage.

---

## Repository

GitHub: https://github.com/TanishRS/tokenvault

## Project Structure

```
tokenvault/
├── include/            # one header per feature (interface + DS justification)
│   ├── Registry.h
│   ├── RevocationTrail.h
│   ├── TransactionLoop.h
│   ├── KeyLookup.h
│   ├── PoolSorter.h
│   ├── LiquidityGraph.h
│   └── SpaceAllocator.h
├── src/                # matching implementations + driver
│   ├── Registry.cpp
│   ├── RevocationTrail.cpp
│   ├── TransactionLoop.cpp
│   ├── KeyLookup.cpp
│   ├── PoolSorter.cpp
│   ├── LiquidityGraph.cpp
│   ├── SpaceAllocator.cpp
│   └── main.cpp
├── samples/
│   └── sample_output.txt
├── screenshots/        # add your execution screenshots here
├── Makefile
├── DESIGN.md           # extended design notes
└── README.md
```
