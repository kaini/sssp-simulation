#pragma once
#include "criteria.hpp"
#include "linear_allocator.hpp"
#include <boost/heap/pairing_heap.hpp>
#include <cfloat>
#include <deque>

namespace sssp {

class traff_bridge : public criteria {
  public:
    traff_bridge(const sssp::graph* graph, size_t start_node);
    virtual void relaxable_nodes(todo_output& output) const override;
    virtual void changed_predecessor(size_t node, size_t predecessor, double distance) override;
    virtual void relaxed_node(size_t node) override;

  private:
    struct node_info;

    struct node_info_compare_distance {
        bool operator()(const node_info* a, const node_info* b) const;
    };

    // tentative(n) - min{ cost(p, n) + min{ cost(p*, p) : p* predecessor of p } : p predecessor of n in U }
    struct node_info_compare_threshold {
        bool operator()(const node_info* a, const node_info* b) const;
    };

    using distance_queue = boost::heap::pairing_heap<node_info*,
                                                     boost::heap::compare<node_info_compare_distance>,
                                                     boost::heap::allocator<local_linear_allocator<node_info*>>>;
    using threshold_queue = boost::heap::pairing_heap<node_info*,
                                                      boost::heap::compare<node_info_compare_threshold>,
                                                      boost::heap::allocator<local_linear_allocator<node_info*>>>;

    struct pred_info {
        pred_info(size_t pred, double cost) : pred(pred), cost(cost) {}
        size_t pred;
        double cost; // cost(p, n) + min{ cost(p*, p) : p* predecessor of p }
    };

    struct node_info {
        node_info(const sssp::graph& graph, size_t index, const local_linear_allocator<pred_info>& pool);
        size_t index;
        double tentative = INFINITY;
        bool settled = false;
        std::deque<pred_info, local_linear_allocator<pred_info>> predecessors;
        distance_queue::handle_type distance_queue_handle;
        threshold_queue::handle_type threshold_queue_handle;

        double threshold() const;
    };

    local_linear_allocator<char> m_pool;
    node_map<node_info> m_info;
    distance_queue m_distance_queue;
    threshold_queue m_threshold_queue;
};

} // namespace sssp