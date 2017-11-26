#pragma once
#include <string>
#include <cstdint>
#include <stdexcept>
#include <iostream>

namespace sssp {

template <typename T> using valid_fn = bool(*)(const T&);
template <typename T> using default_fn = T(*)();

template <typename T, valid_fn<T> Validator, default_fn<T> Default>
class validated {
public:
	validated() : validated(Default()) {}
	validated(T value) : m_value(value) { if (!Validator(value)) throw std::range_error("Invalid value!"); }
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

template <typename T> bool is_positive(const T& value) { return value > 0; }
template <typename T> T return_one() { return T(1); }
using positive_int = validated<int, is_positive<int>, return_one<int>>;
using positive_double = validated<double, is_positive<double>, return_one<double>>;

template <typename T> bool is_from_zero_to_one(const T& value) { return T(0) <= value && value <= T(1); }
template <typename T> T return_zero() { return T(0); }
using from_zero_to_one_double = validated<double, is_from_zero_to_one<double>, return_zero<double>>;

const std::string position_poisson = "poisson";
const std::string position_uniform = "uniform";
inline bool is_position_algorithm(const std::string& s) {
	return s == position_poisson || s == position_uniform;
}
inline std::string default_position_algorithm() {
	return position_poisson;
}
using position_algorithm = validated<std::string, is_position_algorithm, default_position_algorithm>;

const std::string edge_planar = "planar";
const std::string edge_uniform = "uniform";
inline bool is_edge_algorithm(const std::string& s) {
	return s == edge_planar || s == edge_uniform;
}
inline std::string default_edge_algorithm() {
	return edge_planar;
}
using edge_algorithm = validated<std::string, is_edge_algorithm, default_edge_algorithm>;

const std::string cost_uniform = "uniform";
const std::string cost_one = "one";
const std::string cost_euclidean = "euclidean";
inline bool is_cost_algorithm(const std::string& s) {
	return s == cost_uniform || s == cost_one || s == cost_euclidean;
}
inline std::string default_cost_algorithm() {
	return cost_uniform;
}
using cost_algorithm = validated<std::string, is_cost_algorithm, default_cost_algorithm>;

struct arguments {
	struct position_gen {
		position_algorithm algorithm = position_poisson;
		struct poisson {
			positive_double min_distance = 0.1;
			positive_int max_reject = 30;
		} poisson;
		struct uniform {
			positive_int count = 60;
		} uniform;
	} position_gen;

	struct edge_gen {
		edge_algorithm algorithm = edge_planar;
		struct planar {
			positive_double max_length = 0.2;
			from_zero_to_one_double probability = 0.5;
		} planar;
		struct uniform {
			from_zero_to_one_double probability = 0.05;
		} uniform;
	} edge_gen;

	struct cost_gen {
		cost_algorithm algorithm = cost_uniform;
	} cost_gen;

	int seed = 42;
};

arguments parse_arguments(int argc, char* argv[]);

}
