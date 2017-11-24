#pragma once

namespace sssp {

struct rgb {
	double r = 0.0;
	double g = 0.0;
	double b = 0.0;
};

struct vec2 {
	double x = 0.0;
	double y = 0.0;
};

inline bool operator==(const vec2& a, const vec2& b) {
	return a.x == b.x && a.y == b.y;
}

inline bool operator!=(const vec2& a, const vec2& b) {
	return !(a == b);
}

struct line {
	vec2 start;
	vec2 end;
};

double distance(const vec2& a, const vec2& b);
bool intersects(const line& a, const line& b);

}
