#include "heuristic_relaxing_dijkstra.hpp"
#include <boost/assert.hpp>
#include <boost/heap/pairing_heap.hpp>
#include <cfloat>
#include <unordered_set>

namespace {

struct node_info;

struct compare_distance {
    bool operator()(const node_info* a, const node_info* b) const;
};

using node_distance_queue = boost::heap::pairing_heap<node_info*, boost::heap::compare<compare_distance>>;
using incoming_distance_queue = boost::heap::pairing_heap<double, boost::heap::compare<std::greater<double>>>;

enum class node_state {
    unreached,
    queued,
    to_be_relaxed,
    relaxed,
};

struct node_info {
    node_info(size_t index) : index(index) {}

    size_t index;
    incoming_distance_queue incoming_queue; // ordered by estimation + edge cost for unreached or queued nodes

    node_state state = node_state::unreached;
    int relaxation_phase = -1;
    size_t predecessor = -1;
    double distance = INFINITY;

    node_distance_queue::handle_type distance_queue_handle;
    std::unordered_map<size_t, incoming_distance_queue::handle_type> incoming_queue_handles;

    double min_incoming_distance() const {
        if (incoming_queue.empty()) {
            return INFINITY;
        } else {
            return incoming_queue.top();
        }
    }
};

bool compare_distance::operator()(const node_info* a, const node_info* b) const {
    return a->distance > b->distance;
}

} // namespace

sssp::node_map<sssp::dijkstra_result>
sssp::heuristic_relaxing_dijkstra(const graph& graph, size_t start_node, relaxation_heuristic est_fn) {
    node_map<node_info> info = graph.make_node_map([&](size_t i) { return node_info(i); });
    for (node_info& i : info) {
        for (const edge_info& edge : graph.incoming_edges(i.index)) {
            i.incoming_queue_handles[edge.source] = i.incoming_queue.push(est_fn(edge.source) + edge.cost);
        }
    }

    node_distance_queue distance_queue;
    std::vector<node_info*> safe_to_relax;

    info[start_node].distance = 0.0;
    info[start_node].predecessor = -1;
    info[start_node].state = node_state::queued;
    info[start_node].distance_queue_handle = distance_queue.push(&info[start_node]);

    int relaxation_phase = 0;
    while (!distance_queue.empty()) {
        std::unordered_set<node_info*> relax(safe_to_relax.begin(), safe_to_relax.end());
        safe_to_relax.clear();
        relax.emplace(distance_queue.top());
        distance_queue.top()->state = node_state::to_be_relaxed;

        for (node_info* node : relax) {
            BOOST_ASSERT(node->state == node_state::to_be_relaxed);

            node->relaxation_phase = relaxation_phase;
            node->state = node_state::relaxed;
            distance_queue.erase(node->distance_queue_handle);
            for (const edge_info& edge : graph.outgoing_edges(node->index)) {
                node_info& destination = info[edge.destination];
                if (destination.state != node_state::relaxed && destination.state != node_state::to_be_relaxed) {
                    if (node->distance + edge.cost < destination.distance) {
                        destination.distance = node->distance + edge.cost;
                        destination.predecessor = node->index;
                        if (destination.state == node_state::unreached) {
                            destination.state = node_state::queued;
                            destination.distance_queue_handle = distance_queue.push(&destination);
                        } else {
                            BOOST_ASSERT(destination.state == node_state::queued);
                            distance_queue.update(destination.distance_queue_handle);
                        }
                    }

                    destination.incoming_queue.erase(destination.incoming_queue_handles[node->index]);
                    if (destination.distance <= destination.min_incoming_distance()) {
                        safe_to_relax.emplace_back(&destination);
                        destination.state = node_state::to_be_relaxed;
                    }
                }
            }
        }

        relaxation_phase += 1;
    }

    return graph.make_node_map([&](size_t i) {
        dijkstra_result result;
        result.predecessor = info[i].predecessor;
        result.distance = info[i].distance;
        result.relaxation_phase = info[i].relaxation_phase;
        return result;
    });
}
