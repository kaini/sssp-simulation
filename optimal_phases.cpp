#include "optimal_phases.hpp"

sssp::node_map<sssp::dijkstra_result> sssp::optimal_phases(const graph& graph, size_t start_node) {
    node_map<dijkstra_result> reference_solution = dijkstra(graph, start_node);
    node_map<dijkstra_result> result = graph.make_node_map([](size_t n) {
        dijkstra_result r;
        r.distance = INFINITY;
        r.predecessor = -1;
        r.relaxation_phase = -1;
        return r;
    });

    std::vector<size_t> todo;
    todo.emplace_back(start_node);
    result[start_node].distance = 0.0;

    int phase = 0;
    while (todo.size() > 0) {
        std::vector<size_t> new_todo;
        for (size_t source : todo) {
            result[source].relaxation_phase = phase;
            for (const edge_info& edge : graph.outgoing_edges(source)) {
                size_t destination = edge.destination;
                if (result[source].distance + edge.cost < result[destination].distance) {
                    result[destination].distance = result[source].distance + edge.cost;
                    result[destination].predecessor = source;
                    if (source == reference_solution[destination].predecessor) {
                        new_todo.emplace_back(destination);
                    }
                }
            }
        }
        todo = std::move(new_todo);
        phase += 1;
    }

    return result;
}
