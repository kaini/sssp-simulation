#pragma once
#include "math.hpp"
#include <vector>

namespace sssp {

// Generates count positions in the area 0/0 to 1/1 that are uniformly distributed.
// Note that uniform distributions tend to clump or leave big spaces.
std::vector<vec2> generate_uniform_positions(int seed, size_t count);

// This generates a Poisson Disc Sampling in the area 0/0 to 1/1 by using
// Robert Bridson's Fast Poisson Disk Sampling algorithm (2007).
std::vector<vec2> generate_poisson_disc_positions(int seed, double min_distance, int rejection_limit);

}

