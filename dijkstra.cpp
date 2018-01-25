#include "dijkstra.hpp"
#include <boost/assert.hpp>
#include <limits>
#include <vector>

const std::string sssp::dijkstra_result_csv_header("node_count,reachable,relaxation_phases");

std::ostream& sssp::operator<<(std::ostream& out, const sssp::dijkstra_result_csv_values& line) {
    out << line.node_count << "," << line.reachable << "," << line.relaxation_phases;
    return out;
}

sssp::dijkstra_result_csv_values::dijkstra_result_csv_values(const node_map<dijkstra_result>& result)
    : node_count(result.size()),
      reachable(std::count_if(result.begin(), result.end(), [](const auto& n) { return n.distance < INFINITY; })),
      relaxation_phases(
          std::max_element(result.begin(),
                           result.end(),
                           [](const auto& a, const auto& b) { return a.relaxation_phase < b.relaxation_phase; })
              ->relaxation_phase +
          1) {}

sssp::node_map<sssp::dijkstra_result>
sssp::dijkstra(const graph& graph, size_t start_node, boost::base_collection<criteria>& criteria) {
    node_map<dijkstra_result> info = graph.make_node_map([](size_t i) { return dijkstra_result(); });
    int current_phase = 0;

    // Put the start node in the fringe set.
    info[start_node].distance = 0.0;
    for (auto& crit : criteria) {
        crit.changed_predecessor(start_node, size_t(-1), 0.0);
    }

    std::vector<size_t> todo;
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

        // Relax each node.
        for (size_t node : todo) {
            dijkstra_result& current_node = info[node];
            if (!current_node.settled()) {
                current_node.relaxation_phase = current_phase;

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
        }

        // Next round.
        ++current_phase;
    }

    return info;
}
