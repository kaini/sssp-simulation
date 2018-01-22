#pragma once
#include "criteria.hpp"
#include <boost/heap/pairing_heap.hpp>
#include <functional>
#include <unordered_set>

namespace sssp {

using relaxation_heuristic = std::function<double(size_t node)>;

// Uses a heuristic to find nodes to relax.
// The heuristic has to *underestimate* the real cost, i.e., est_fn(node) <= distance(start, node).
class heuristic : public criteria {
  public:
    heuristic(const sssp::graph* graph, size_t start_node, relaxation_heuristic heuristic);
    virtual void relaxable_nodes(todo_output& output) const override;
    virtual void changed_predecessor(size_t node, size_t predecessor, double distance) override;
    virtual void relaxed_node(size_t node) override;

  private:
    struct pred_info {
        pred_info(size_t index, double est_distance) : index(index), estimated_distance(est_distance) {}
        size_t index;
        double estimated_distance;
    };

    struct node_info {
        node_info(const sssp::graph& graph, relaxation_heuristic h, size_t index);
        size_t index;
        std::vector<pred_info> unsettled_predecessors;
        double tentative_distance = INFINITY;
        bool settled = false;
    };

    relaxation_heuristic m_heuristic;
    std::unordered_set<size_t> m_safe_to_relax;
    node_map<node_info> m_node_info;
};

} // namespace sssp