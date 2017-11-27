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

using node_distance_queue = boost::heap::pairing_heap<node_info*, boost::heap::compare<compare_distance>>;
using node_distance_out_queue = boost::heap::pairing_heap<node_info*, boost::heap::compare<compare_distance_out>>;
using node_distance_in_queue = boost::heap::pairing_heap<node_info*, boost::heap::compare<compare_distance_in>>;

enum class node_state {
    unreached,
    queued,
    relaxed,
};

struct node_info {
    node_info(size_t index, double min_outgoing_cost, double min_incoming_cost)
        : index(index), min_outgoing_cost(min_outgoing_cost), min_incoming_cost(min_incoming_cost){};

    size_t index;
    double min_outgoing_cost;
    double min_incoming_cost;

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
    return a->distance + a->min_outgoing_cost > b->distance + b->min_outgoing_cost;
}

bool compare_distance_in::operator()(const node_info* a, const node_info* b) const {
    return a->distance - a->min_incoming_cost > b->distance - b->min_incoming_cost;
}

} // namespace

sssp::node_map<sssp::dijkstra_result> sssp::crauser(const graph& graph, size_t start_node, crauser_criteria criteria) {
    node_map<node_info> info = graph.make_node_map([&](size_t i) {
        double min_outgoing_cost = INFINITY;
        const auto& outgoing_edges = graph.outgoing_edges(i);
        if (outgoing_edges.size()) {
            min_outgoing_cost = std::min_element(outgoing_edges.begin(),
                                                 outgoing_edges.end(),
                                                 [](const auto& a, const auto& b) { return a.cost < b.cost; })
                                    ->cost;
        }

        double min_incoming_cost = INFINITY;
        const auto& incoming_edges = graph.incoming_edges(i);
        if (incoming_edges.size()) {
            min_incoming_cost = std::min_element(incoming_edges.begin(),
                                                 incoming_edges.end(),
                                                 [](const auto& a, const auto& b) { return a.cost < b.cost; })
                                    ->cost;
        }

        return node_info(i, min_outgoing_cost, min_incoming_cost);
    });
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
            double threshold = min_out_node->distance + min_out_node->min_outgoing_cost;
            while (distance_queue.size() > 0 && distance_queue.top()->distance <= threshold) {
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
            while (distance_in_queue.size() > 0 &&
                   distance_in_queue.top()->distance - distance_in_queue.top()->min_incoming_cost <= current_distance) {
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
