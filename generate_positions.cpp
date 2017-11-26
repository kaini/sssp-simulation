#include "generate_positions.hpp"
#include <algorithm>
#include <cmath>
#include <random>

std::vector<sssp::vec2> sssp::generate_uniform_positions(int seed, size_t count) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> uniform_coordinate(0.0, 1.0);

    std::vector<vec2> result;
    result.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        result.emplace_back(vec2{uniform_coordinate(rng), uniform_coordinate(rng)});
    }
    return result;
}

std::vector<sssp::vec2> sssp::generate_poisson_disc_positions(int seed, double min_distance, int rejection_limit) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> uniform_coordinate(0.0, 1.0);
    std::uniform_real_distribution<double> uniform_angle(0, 2 * M_PI);
    std::uniform_real_distribution<double> uniform_squared_radius(min_distance * min_distance,
                                                                  4 * min_distance * min_distance);

    double cell_size = min_distance / std::sqrt(2.0);
    size_t cells = size_t(std::ceil(1.0 / cell_size));
    std::vector<size_t> grid(cells * cells, -1);
    std::vector<size_t> active_list;
    std::vector<sssp::vec2> samples;

    samples.emplace_back(vec2{uniform_coordinate(rng), uniform_coordinate(rng)});
    grid[size_t(std::floor(samples[0].x / cell_size)) + size_t(std::floor(samples[0].y / cell_size)) * cells] = 0;
    active_list.emplace_back(0);

    while (active_list.size() > 0) {
        size_t active_list_i = std::uniform_int_distribution<size_t>(0, active_list.size() - 1)(rng);
        sssp::vec2 root_sample = samples[active_list[active_list_i]];

        int k = 0;
        for (; k < rejection_limit; ++k) {
            // Draw a sample from the annulus r to 2r.
            // https://stackoverflow.com/questions/9048095/create-random-number-within-an-annulus
            double theta = uniform_angle(rng);
            double radius = std::sqrt(uniform_squared_radius(rng));
            sssp::vec2 sample{root_sample.x + radius * std::cos(theta), root_sample.y + radius * std::sin(theta)};
            if (sample.x < 0.0 || sample.x >= 1.0 || sample.y < 0.0 || sample.y >= 1.0) {
                continue;
            }

            size_t sample_cell_x = size_t(std::floor(sample.x / cell_size));
            size_t sample_cell_y = size_t(std::floor(sample.y / cell_size));
            bool valid = true;
            for (int xx = -2; xx <= 2 && valid; ++xx) {
                if (0 <= sample_cell_x + xx && sample_cell_x + xx < cells) {
                    for (int yy = -2; yy <= 2 && valid; ++yy) {
                        if (0 <= sample_cell_y + yy && sample_cell_y + yy < cells) {
                            size_t grid_value = grid[(sample_cell_x + xx) + (sample_cell_y + yy) * cells];
                            if (grid_value != -1 && distance(samples[grid_value], sample) < min_distance) {
                                valid = false;
                            }
                        }
                    }
                }
            }

            if (valid) {
                grid[sample_cell_x + sample_cell_y * cells] = samples.size();
                active_list.emplace_back(samples.size());
                samples.emplace_back(sample);
                break;
            }
        }

        if (k == rejection_limit) {
            active_list.erase(active_list.begin() + active_list_i);
        }
    }

    return samples;
}
