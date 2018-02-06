#include "dijkstra.hpp"
#include <boost/assert.hpp>
#include <limits>

sssp::node_map<sssp::dijkstra_result>
sssp::dijkstra(const graph& graph, size_t start_node, boost::base_collection<criteria>& criteria) {
    node_map<dijkstra_result> info = graph.make_node_map([](size_t i) { return dijkstra_result(); });
    int current_phase = 0;

    // Put the start node in the fringe set.
    info[start_node].distance = 0.0;
    for (auto& crit : criteria) {
        crit.changed_predecessor(start_node, size_t(-1), 0.0);
    }

    criteria::todo_output todo;
    todo.reserve(graph.node_count());
    while (true) {
        // Find nodes to be relaxed.
        todo.clear();
        for (auto& crit : criteria) {
            crit.relaxable_nodes(todo);
        }

        // If there is nothing more to do, we are done.
        if (todo.empty()) {
            break;
        }

        // First all todo nodes will be set to settled. This exposes some errors
        // and simulates the parallel relaxation better. If this does not happen
        // now but later in the loop, the arbritary relaxation order might hide
        // errors.
        for (size_t node : todo) {
            info[node].relaxation_phase = current_phase;
        }

        // Relax each node.
        for (size_t node : todo) {
            dijkstra_result& current_node = info[node];
            BOOST_ASSERT(current_node.settled()); // set above

            for (const edge_info& edge : graph.outgoing_edges(node)) {
                dijkstra_result& destination_node = info[edge.destination];
                if (!destination_node.settled() && current_node.distance + edge.cost < destination_node.distance) {
                    destination_node.distance = current_node.distance + edge.cost;
                    destination_node.predecessor = node;
                    for (auto& crit : criteria) {
                        crit.changed_predecessor(edge.destination, node, destination_node.distance);
                    }
                }
            }

            for (auto& crit : criteria) {
                crit.relaxed_node(node);
            }
        }

        // Next round.
        ++current_phase;
    }

    return info;
}
