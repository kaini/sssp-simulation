#pragma once
#include "criteria.hpp"
#include "dijkstra.hpp"
#include "graph.hpp"
#include "stringy_enum.hpp"
#include <boost/heap/pairing_heap.hpp>
#include <unordered_set>

namespace sssp {

// Implements Crauser's IN criteria. Additionally instead of using the minimal edges,
// with dynamic=true, one can use the minimal non-settled edge.
class crauser_in : public criteria {
  public:
    crauser_in(const sssp::graph* graph, size_t start_node, bool dynamic);
    virtual std::unordered_set<size_t> relaxable_nodes() const override;
    virtual void changed_predecessor(size_t node, size_t predecessor, double distance) override;
    virtual void relaxed_node(size_t node) override;
    bool dynamic() const { return m_dynamic; }

  private:
    struct node_info;
    struct node_info_compare_distance {
        bool operator()(const node_info* a, const node_info* b) const;
    };
    // threshold(n) = tentative(n) - min{ cost of incmoing edges (not settled if dynamic) }
    // Called "i" in the paper.
    struct node_info_compare_threshold {
        bool operator()(const node_info* a, const node_info* b) const;
    };

    using distance_queue = boost::heap::pairing_heap<node_info*, boost::heap::compare<node_info_compare_distance>>;
    using threshold_queue = boost::heap::pairing_heap<node_info*, boost::heap::compare<node_info_compare_threshold>>;

    struct node_info {
        node_info(const sssp::graph& g, size_t index);
        size_t index;
        std::vector<edge_info> incoming;
        double tentative_distance = INFINITY;
        bool settled = false;
        distance_queue::handle_type distance_queue_handle;
        threshold_queue::handle_type threshold_queue_handle;

        double threshold() const;
    };

    bool m_dynamic;
    node_map<node_info> m_node_info;
    distance_queue m_distance_queue;
    threshold_queue m_threshold_queue;
};

// Implements Crauser's OUT criteria. Additionally instead of using the minimal edges,
// with dynamic=true, one can use the minimal non-settled edge.
class crauser_out : public criteria {
  public:
    crauser_out(const sssp::graph* graph, size_t start_node, bool dynamic);
    virtual std::unordered_set<size_t> relaxable_nodes() const override;
    virtual void changed_predecessor(size_t node, size_t predecessor, double distance) override;
    virtual void relaxed_node(size_t node) override;
    bool dynamic() const { return m_dynamic; }

  private:
    struct node_info;
    struct node_info_compare_distance {
        bool operator()(const node_info* a, const node_info* b) const;
    };
    // threshold(n) = tentative(n) + min{ outgoing edge (not settled if dynamic) }
    // Called "L" in the paper (to be exact this are the values of all nodes, not the final L)
    struct node_info_compare_threshold {
        bool operator()(const node_info* a, const node_info* b) const;
    };

    using distance_queue = boost::heap::pairing_heap<node_info*, boost::heap::compare<node_info_compare_distance>>;
    using threshold_queue = boost::heap::pairing_heap<node_info*, boost::heap::compare<node_info_compare_threshold>>;

    struct node_info {
        node_info(const sssp::graph& g, size_t index);
        size_t index;
        std::vector<edge_info> outgoing;
        double tentative_distance = INFINITY;
        bool settled = false;
        distance_queue::handle_type distance_queue_handle;
        threshold_queue::handle_type threshold_queue_handle;

        double threshold() const;
    };

    bool m_dynamic;
    node_map<node_info> m_node_info;
    distance_queue m_distance_queue;
    threshold_queue m_threshold_queue;
};

} // namespace sssp