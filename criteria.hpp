#pragma once
#include "graph.hpp"
#include <unordered_set>

namespace sssp {

class criteria {
  public:
    using todo_output = std::unordered_set<size_t>;

    criteria(const graph* graph, size_t start_node) : m_graph(graph), m_start_node(start_node) {}

    virtual ~criteria() = default;

    // Called at the start of each relaxation phase to find nodes to relax.
    virtual void relaxable_nodes(todo_output& output) const = 0;

    // Called when a node in the fringe or unexplored set gets a better predecessor
    // assigned. Called before the respective relaxed_node.
    virtual void changed_predecessor(size_t node, size_t predecessor, double distance) = 0;

    // Called for each node that is being relaxed. Note that no all nodes returned
    // by relaxable_nodes might be relaxed and note that this might be called
    // for nodes not returned by relaxable_nodes. Called after the appropriate
    // changed_predecessor calls.
    virtual void relaxed_node(size_t node) = 0;

    // Return true if the criteria alone is complete.
    virtual bool is_complete() const = 0;

  protected:
    const sssp::graph& graph() const { return *m_graph; }
    size_t start_node() const { return m_start_node; }

  private:
    const sssp::graph* m_graph;
    size_t m_start_node;
};

} // namespace sssp
