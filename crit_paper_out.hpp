#pragma once
#include "criteria.hpp"
#include "graph.hpp"

namespace sssp {

class paper_out : public criteria {
  public:
    paper_out(const sssp::graph* graph, size_t start_node);
    virtual void relaxable_nodes(todo_output& output) const override;
    virtual void changed_predecessor(size_t node, size_t predecessor, double distance) override;
    virtual void relaxed_node(size_t node) override;
    virtual bool is_complete() const override { return true; }

  private:
    struct node_info {
        bool settled = false;
        double tentative_distance = INFINITY;
    };

    node_map<node_info> m_node_info;
};

} // namespace sssp
