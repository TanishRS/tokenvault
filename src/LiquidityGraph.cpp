#include "LiquidityGraph.h"
#include <queue>
#include <limits>
#include <algorithm>

int LiquidityGraph::id(const std::string& pool) const {
    auto it = id_.find(pool);
    return it == id_.end() ? -1 : it->second;
}

int LiquidityGraph::addPool(const std::string& pool) {
    auto it = id_.find(pool);
    if (it != id_.end()) return it->second;
    int idx = static_cast<int>(name_.size());
    id_[pool] = idx;
    name_.push_back(pool);
    adj_.emplace_back();
    return idx;
}

void LiquidityGraph::addSwap(const std::string& a, const std::string& b,
                             double risk, double fee, bool bidirectional) {
    int ia = addPool(a), ib = addPool(b);
    double w = risk + fee;                 // combined edge cost
    adj_[ia].push_back({ib, w});
    if (bidirectional) adj_[ib].push_back({ia, w});
}

LiquidityGraph::RouteResult
LiquidityGraph::lowestRiskRoute(const std::string& src, const std::string& dst) const {
    RouteResult res;
    int s = id(src), d = id(dst);
    if (s < 0 || d < 0) return res;        // unknown pool

    const double INF = std::numeric_limits<double>::infinity();
    std::vector<double> dist(name_.size(), INF);
    std::vector<int>    prev(name_.size(), -1);
    dist[s] = 0.0;

    // min-heap: (cost, node)
    using PQItem = std::pair<double,int>;
    std::priority_queue<PQItem, std::vector<PQItem>, std::greater<PQItem>> pq;
    pq.push({0.0, s});

    while (!pq.empty()) {
        auto [cost, u] = pq.top(); pq.pop();
        if (cost > dist[u]) continue;      // stale entry
        if (u == d) break;                 // reached target
        for (const Edge& e : adj_[u]) {
            double nd = cost + e.weight;
            if (nd < dist[e.to]) {
                dist[e.to] = nd;
                prev[e.to] = u;
                pq.push({nd, e.to});
            }
        }
    }

    if (dist[d] == INF) return res;        // unreachable
    res.found = true;
    res.totalCost = dist[d];
    for (int at = d; at != -1; at = prev[at]) res.path.push_back(name_[at]);
    std::reverse(res.path.begin(), res.path.end());
    return res;
}
