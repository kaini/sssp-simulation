#pragma once
#include <string>
#include <cstdint>

namespace sssp {

template <typename T> using validator_fn = bool(*)(const T&);
template <typename T> using default_fn = T(*)();

template <typename T, validator_fn<T> Validator, default_fn<T> Default>
class validated {
public:
	validated() : validated(Default()) {}
	validated(T value) : m_value(value) { if (!Validator(value)) throw std::range_error("Invalid value!"); }
	T value() const { return m_value; }
	operator T() const { return m_value; }
private:
	T m_value;
};

template <typename T, validator_fn<T> Validator, default_fn<T> Default>
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

template <typename T, validator_fn<T> Validator, default_fn<T> Default>
std::ostream& operator<<(std::ostream& out, const validated<T, Validator, Default>& value) {
	out << value.value();
	return out;
}

template <typename T, T Arg> T return_arg() { return Arg; }

template <typename T> bool is_positive(const T& value) { return value > T(0); }
template <typename T> T return_max() { return std::numeric_limits<T>::max(); }
template <typename T> using positive = validated<T, is_positive<T>, return_max<T>>;

template <typename T, T Min, T Max> bool is_bounded(const T& value) { return Min <= value && value <= Max; }
template <typename T, T Min, T Max> using bounded = validated<T, is_bounded<T, Min, Max>, return_arg<T, Min>>;

const std::string position_poisson = "poisson";
const std::string position_uniform = "uniform";
inline bool is_position_algorithm(const std::string& s) {
	return s == position_poisson || s == position_uniform;
}
inline std::string default_position_algorithm() {
	return position_poisson;
}
using position_algorithm = validated<std::string, is_position_algorithm, default_position_algorithm>;

struct arguments {
	struct position_gen {
		position_algorithm algorithm = position_poisson;
		struct poisson {
			positive<double> min_distance = 0.1;
			bounded<int, 1, INT_MAX> max_reject = 30;
		} poisson;
		struct uniform {
			bounded<int, 1, INT_MAX> count = 60;
		} uniform;
	} position_gen;

	int seed = 42;
};

arguments parse_arguments(int argc, char* argv[]);

}
