#include "dijkstra.hpp"
#include <vector>
#include <limits>
#include <boost/assert.hpp>
#include <boost/heap/pairing_heap.hpp>

namespace {

struct node_info;

struct compare_distance {
	bool operator()(const node_info* a, const node_info* b) const;
};

using node_queue = boost::heap::pairing_heap<node_info*, boost::heap::compare<compare_distance>>;

enum class node_state {
	unreached,
	queued,
	relaxed,
};

struct node_info {
	node_info(size_t index) : index(index) {};

	size_t index;
	node_state state = node_state::unreached;
	size_t predecessor = -1;
	double distance = INFINITY;
	node_queue::handle_type queue_handle;
};

bool compare_distance::operator()(const node_info* a, const node_info* b) const {
	return a->distance > b->distance;
}

}

void sssp::dijkstra(const graph& graph, size_t start_node) {
	std::vector<node_info> info;
	info.reserve(graph.node_count());
	for (size_t i = 0; i < graph.node_count(); ++i) {
		info.emplace_back(node_info(i));
	}

	node_queue queue;

	info[start_node].state = node_state::queued;
	info[start_node].distance = 0.0;
	info[start_node].queue_handle = queue.push(&info[start_node]);

	while (!queue.empty()) {
		node_info& current_node = *queue.top();
		queue.pop();
		current_node.state = node_state::relaxed;

		for (const edge_info& edge : graph.outgoing_edges(current_node.index)) {
			node_info& destination_node = info[edge.destination];
			if (destination_node.state != node_state::relaxed && current_node.distance + edge.cost < destination_node.distance) {
				destination_node.distance = current_node.distance + edge.cost;
				destination_node.predecessor = current_node.index;
				if (destination_node.state == node_state::unreached) {
					destination_node.state = node_state::queued;
					destination_node.queue_handle = queue.push(&destination_node);
				} else {
					BOOST_ASSERT(destination_node.state == node_state::queued);
					// Note: Boost's heaps are max-heaps and we compare using operator>.
					// Therefore if an element is decreased in value it is increased in the heap.
					queue.increase(destination_node.queue_handle);
				}
			}
		}
	}
}
