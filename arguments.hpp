#pragma once
#include "stringy_enum.hpp"
#include <boost/assert.hpp>
#include <boost/optional.hpp>
#include <cstdint>
#include <stdexcept>
#include <unordered_set>
#include <vector>

namespace sssp {

STRINGY_ENUM(position_algorithm, poisson, uniform)
STRINGY_ENUM(edge_algorithm, planar, uniform, layered)
STRINGY_ENUM(cost_algorithm, uniform, one, euclidean)
STRINGY_ENUM(sssp_algorithm,
             dijkstra,
             crauser_in,
             crauser_in_dyn,
             crauser_out,
             crauser_out_dyn,
             oracle,
             heuristic,
             traff)

std::ostream& operator<<(std::ostream& out, const std::vector<sssp_algorithm>& algorithms);

template <typename T> using valid_fn = bool (*)(const T&);
template <typename T> using default_fn = T (*)();

template <typename T, valid_fn<T> Validator, default_fn<T> Default> class validated {
  public:
    validated() : validated(Default()) {}
    validated(T value) : m_value(value) {
        if (!Validator(value))
            throw std::range_error("Invalid value!");
    }
    T value() const { return m_value; }
    operator T() const { return m_value; }

  private:
    T m_value;
};

template <typename T, valid_fn<T> Validator, default_fn<T> Default>
std::istream& operator>>(std::istream& in, validated<T, Validator, Default>& value) {
    T raw_value;
    in >> raw_value;
    if (!in.fail() && !in.bad()) {
        if (Validator(raw_value)) {
            value = raw_value;
        } else {
            in.setstate(std::ios_base::failbit);
        }
    }
    return in;
}

template <typename T, valid_fn<T> Validator, default_fn<T> Default>
std::ostream& operator<<(std::ostream& out, const validated<T, Validator, Default>& value) {
    out << value.value();
    return out;
}

template <typename T> bool is_positive(const T& value) {
    return value > 0;
}
template <typename T> T return_one() {
    return T(1);
}
using positive_int = validated<int, is_positive<int>, return_one<int>>;
using positive_double = validated<double, is_positive<double>, return_one<double>>;

template <typename T> bool is_from_zero_to_one(const T& value) {
    return T(0) <= value && value <= T(1);
}
template <typename T> T return_zero() {
    return T(0);
}
using from_zero_to_one_double = validated<double, is_from_zero_to_one<double>, return_zero<double>>;

struct arguments {
    struct position_gen {
        position_algorithm algorithm = position_algorithm::poisson;
        struct poisson {
            positive_double min_distance = 0.1;
            positive_int max_reject = 30;
        } poisson;
        struct uniform {
            positive_int count = 60;
        } uniform;
    } position_gen;

    struct edge_gen {
        edge_algorithm algorithm = edge_algorithm::planar;
        struct planar {
            from_zero_to_one_double probability = 0.05;
        } planar;
        struct uniform {
            from_zero_to_one_double probability = 0.05;
        } uniform;
        struct layered {
            from_zero_to_one_double probability = 0.10;
            positive_int count = 2;
        } layered;
    } edge_gen;

    struct cost_gen {
        cost_algorithm algorithm = cost_algorithm::uniform;
    } cost_gen;

    int seed = 42;
    std::vector<sssp_algorithm> algorithms = {sssp_algorithm::dijkstra};
    positive_int runs = 1;
#ifndef DISABLE_CAIRO
    std::string image = "";
#endif
};

extern const std::string arguments_csv_header;
std::string arguments_csv_values(const arguments& args);

boost::optional<arguments> parse_arguments(int argc, char* argv[]);

} // namespace sssp
