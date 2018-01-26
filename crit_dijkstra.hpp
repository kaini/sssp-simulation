#pragma once
#include "criteria.hpp"
#include <boost/heap/pairing_heap.hpp>

namespace sssp {

class smallest_tentative_distance : public criteria {
  public:
    smallest_tentative_distance(const sssp::graph* graph, size_t start_node);
    virtual void relaxable_nodes(todo_output& output) const override;
    virtual void changed_predecessor(size_t node, size_t predecessor, double distance) override;
    virtual void relaxed_node(size_t node) override;

  private:
    struct node_info;

    struct node_info_compare {
        bool operator()(const node_info* a, const node_info* b) const;
    };

    using queue = boost::heap::pairing_heap<node_info*, boost::heap::compare<node_info_compare>>;

    struct node_info {
        node_info(size_t index) : index(index) {}
        size_t index;
        double tentative_distance = INFINITY;
        queue::handle_type queue_handle;
    };

    node_map<node_info> m_node_info;
    queue m_queue;
};

} // namespace sssp