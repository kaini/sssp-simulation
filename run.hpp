#pragma once
#include "arguments.hpp"
#include <iostream>
#include <string>

namespace sssp {

extern const std::string dijkstra_result_csv_header;

// Executes a run. This also saves the image file if request.
// Only outputs to the given ostream.
void execute_run(const arguments& arguments, std::ostream* out, std::ostream* err);

} // namespace sssp
