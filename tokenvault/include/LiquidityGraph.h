#ifndef LIQUIDITY_GRAPH_H
#define LIQUIDITY_GRAPH_H

// ============================================================================
//  Connection Map  -->  graph (custom adjacency list)
//  Route Path      -->  Dijkstra over that graph
// ----------------------------------------------------------------------------
//  Each liquidity pool is a NODE. Each swap pair is an EDGE whose WEIGHT is
//  the combined risk + fee of taking that swap. "Lowest risk/fee route, even
//  through multiple intermediate swaps" = shortest path on a weighted graph.
//
//  DS CHOICE: adjacency list (vector of edge-lists), NOT an adjacency matrix.
//    - Liquidity graphs are SPARSE (a pool connects to a handful of others,
//      not all millions). Adjacency list = O(V + E) space vs O(V^2) matrix.
//    - Dijkstra with a min-heap (priority_queue) = O((V + E) log V), correct
//      because all weights (risk+fee) are non-negative.
//    - BFS would give fewest-hops, not lowest-cost; Bellman-Ford handles
//      negative edges we don't have and is slower. Dijkstra is the right fit.
//  REAL WORLD: this is exactly how a DEX aggregator (1inch / Uniswap routing)
//  finds the best multi-hop swap path across pools.
//
//  This is hand-built (no STL graph) because the graph + pathfinding is the
//  part of the brief actually worth implementing, not just selecting.
// ============================================================================

#include <string>
#include <vector>
#include <unordered_map>

class LiquidityGraph {
public:
    // Register a pool (node). Idempotent. O(1) average.
    int addPool(const std::string& pool);

    // Add a swap pair (edge). Directed cost = risk + fee. Undirected by
    // default (a swap usually works both ways). O(1) amortized.
    void addSwap(const std::string& a, const std::string& b,
                 double risk, double fee, bool bidirectional = true);

    struct RouteResult {
        bool                     found = false;
        double                   totalCost = 0.0;
        std::vector<std::string> path;        // src ... dst
    };

    // Lowest-cost (risk+fee) route from src to dst via Dijkstra.
    RouteResult lowestRiskRoute(const std::string& src, const std::string& dst) const;

    size_t poolCount() const { return id_.size(); }

private:
    struct Edge { int to; double weight; };

    int id(const std::string& pool) const;            // -1 if absent

    std::unordered_map<std::string,int>  id_;          // name -> index
    std::vector<std::string>             name_;        // index -> name
    std::vector<std::vector<Edge>>       adj_;         // adjacency list
};

#endif // LIQUIDITY_GRAPH_H
