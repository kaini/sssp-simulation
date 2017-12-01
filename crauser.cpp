#include "crauser.hpp"
#include <boost/assert.hpp>
#include <boost/heap/pairing_heap.hpp>

namespace {

struct node_info;

struct compare_distance {
    bool operator()(const node_info* a, const node_info* b) const;
};

struct compare_distance_out {
    bool operator()(const node_info* a, const node_info* b) const;
};

struct compare_distance_in {
    bool operator()(const node_info* a, const node_info* b) const;
};

struct compare_cost {
    bool operator()(const sssp::edge_info& a, const sssp::edge_info& b) const { return a.cost > b.cost; }
};

using node_distance_queue = boost::heap::pairing_heap<node_info*, boost::heap::compare<compare_distance>>;
using node_distance_out_queue = boost::heap::pairing_heap<node_info*, boost::heap::compare<compare_distance_out>>;
using node_distance_in_queue = boost::heap::pairing_heap<node_info*, boost::heap::compare<compare_distance_in>>;

enum class node_state {
    unreached,
    queued,
    relaxed,
};

struct node_info {
    node_info(size_t index, const sssp::graph& graph) : index(index) {
        outgoing_edges = graph.outgoing_edges(index);
        std::sort(outgoing_edges.begin(), outgoing_edges.end(), compare_cost());
        incoming_edges = graph.incoming_edges(index);
        std::sort(incoming_edges.begin(), incoming_edges.end(), compare_cost());
    }

    double min_outgoing_cost() const {
        if (outgoing_edges.empty()) {
            return INFINITY;
        } else {
            return outgoing_edges.back().cost;
        }
    }

    double min_incoming_cost() const {
        if (incoming_edges.empty()) {
            return INFINITY;
        } else {
            return incoming_edges.back().cost;
        }
    }

    size_t index;
    std::vector<sssp::edge_info> outgoing_edges; // sorted by cost descending
    std::vector<sssp::edge_info> incoming_edges; // sorted by cost descending

    node_state state = node_state::unreached;
    size_t predecessor = -1;
    int relaxation_phase = -1;
    double distance = INFINITY;

    node_distance_queue::handle_type distance_queue_handle;
    node_distance_out_queue::handle_type distance_out_queue_handle;
    node_distance_in_queue::handle_type distance_in_queue_handle;
};

bool compare_distance::operator()(const node_info* a, const node_info* b) const {
    return a->distance > b->distance;
}

bool compare_distance_out::operator()(const node_info* a, const node_info* b) const {
    return a->distance + a->min_outgoing_cost() > b->distance + b->min_outgoing_cost();
}

bool compare_distance_in::operator()(const node_info* a, const node_info* b) const {
    return a->distance - a->min_incoming_cost() > b->distance - b->min_incoming_cost();
}

} // namespace

sssp::node_map<sssp::dijkstra_result>
sssp::crauser(const graph& graph, size_t start_node, crauser_criteria criteria, bool dynamic) {
    node_map<node_info> info = graph.make_node_map([&](size_t i) { return node_info(i, graph); });
    node_distance_queue distance_queue;         // ordered by distance
    node_distance_out_queue distance_out_queue; // ordered by distance + cheapest outgoing edge
    node_distance_in_queue distance_in_queue;   // ordered by distance - cheapest incoming edge

    info[start_node].state = node_state::queued;
    info[start_node].distance = 0.0;
    info[start_node].distance_queue_handle = distance_queue.push(&info[start_node]);
    info[start_node].distance_out_queue_handle = distance_out_queue.push(&info[start_node]);
    info[start_node].distance_in_queue_handle = distance_in_queue.push(&info[start_node]);

    int current_phase = 0;
    while (!distance_queue.empty()) {
        BOOST_ASSERT(distance_queue.size() == distance_out_queue.size());
        BOOST_ASSERT(distance_queue.size() == distance_in_queue.size());

        std::vector<node_info*> current_nodes;
        double current_distance = distance_queue.top()->distance;

        // OUT criteria
        if (criteria == crauser_criteria::out || criteria == crauser_criteria::inout) {
            node_info* min_out_node = distance_out_queue.top();
            double threshold = min_out_node->distance + min_out_node->min_outgoing_cost();
            while (!distance_queue.empty() && distance_queue.top()->distance <= threshold) {
                node_info* node = distance_queue.top();
                node->state = node_state::relaxed;
                node->relaxation_phase = current_phase;
                current_nodes.emplace_back(node);
                distance_out_queue.erase(node->distance_out_queue_handle);
                distance_in_queue.erase(node->distance_in_queue_handle);
                distance_queue.pop();
            }
        }

        // IN criteria
        if (criteria == crauser_criteria::in || criteria == crauser_criteria::inout) {
            while (!distance_in_queue.empty() &&
                   distance_in_queue.top()->distance - distance_in_queue.top()->min_incoming_cost() <=
                       current_distance) {
                node_info* node = distance_in_queue.top();
                node->state = node_state::relaxed;
                node->relaxation_phase = current_phase;
                current_nodes.emplace_back(node);
                distance_out_queue.erase(node->distance_out_queue_handle);
                distance_queue.erase(node->distance_queue_handle);
                distance_in_queue.pop();
            }
        }

        BOOST_ASSERT(current_nodes.size() > 0);

        for (node_info* current_node : current_nodes) {
            for (const edge_info& edge : graph.outgoing_edges(current_node->index)) {
                node_info& destination_node = info[edge.destination];
                if (destination_node.state != node_state::relaxed &&
                    current_node->distance + edge.cost < destination_node.distance) {
                    destination_node.distance = current_node->distance + edge.cost;
                    destination_node.predecessor = current_node->index;
                    if (destination_node.state == node_state::unreached) {
                        destination_node.state = node_state::queued;
                        destination_node.distance_queue_handle = distance_queue.push(&destination_node);
                        destination_node.distance_out_queue_handle = distance_out_queue.push(&destination_node);
                        destination_node.distance_in_queue_handle = distance_in_queue.push(&destination_node);
                    } else {
                        BOOST_ASSERT(destination_node.state == node_state::queued);
                        distance_queue.update(destination_node.distance_queue_handle);
                        distance_out_queue.update(destination_node.distance_out_queue_handle);
                        distance_in_queue.update(destination_node.distance_in_queue_handle);
                    }
                }
            }
        }

        if (dynamic) {
            for (node_info* current_node : current_nodes) {
                for (const edge_info& out_edge : graph.outgoing_edges(current_node->index)) {
                    node_info& dest_node = info[out_edge.destination];
                    if (dest_node.state != node_state::relaxed) {
                        while (!dest_node.incoming_edges.empty() &&
                               info[dest_node.incoming_edges.back().source].state == node_state::relaxed) {
                            dest_node.incoming_edges.pop_back();
                        }
                        if (dest_node.state == node_state::queued) {
                            distance_in_queue.update(dest_node.distance_in_queue_handle);
                        }
                    }
                }
                for (const edge_info& in_edge : graph.incoming_edges(current_node->index)) {
                    node_info& src_node = info[in_edge.source];
                    if (src_node.state != node_state::relaxed) {
                        while (!src_node.outgoing_edges.empty() &&
                               info[src_node.outgoing_edges.back().destination].state == node_state::relaxed) {
                            src_node.outgoing_edges.pop_back();
                        }
                        if (src_node.state == node_state::queued) {
                            distance_out_queue.update(src_node.distance_out_queue_handle);
                        }
                    }
                }
            }
        }

        ++current_phase;
    }

    BOOST_ASSERT(distance_out_queue.size() == 0);
    BOOST_ASSERT(distance_in_queue.size() == 0);

    return graph.make_node_map([&](size_t i) {
        dijkstra_result result;
        result.predecessor = info[i].predecessor;
        result.distance = info[i].distance;
        result.relaxation_phase = info[i].relaxation_phase;
        return result;
    });
}
