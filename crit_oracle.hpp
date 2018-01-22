#pragma once
#include "criteria.hpp"
#include "dijkstra.hpp"
#include <cstdlib>
#include <unordered_map>

namespace sssp {

// Relaxes each node that is reached via its shortest path.
class oracle : public criteria {
  public:
    oracle(const sssp::graph* graph, size_t start_node);
    virtual void relaxable_nodes(std::unordered_set<size_t>& output) const override;
    virtual void changed_predecessor(size_t node, size_t predecessor, double distance) override;
    virtual void relaxed_node(size_t node) override;

  private:
    node_map<dijkstra_result> m_result;
    std::unordered_map<size_t, double> m_fringe;
};

} // namespace sssp
