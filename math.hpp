#pragma once

namespace sssp {

struct rgb {
	rgb() : r(0.0), g(0.0), b(0.0) {}
	rgb(double r, double g, double b) : r(r), g(g), b(b) {}
	double r;
	double g;
	double b;
};

struct vec2 {
	vec2() : x(0.0), y(0.0) {}
	vec2(double x, double y) : x(x), y(y) {}
	double x;
	double y;
};

inline bool operator==(const vec2& a, const vec2& b) {
	return a.x == b.x && a.y == b.y;
}

inline bool operator!=(const vec2& a, const vec2& b) {
	return !(a == b);
}

struct line {
	line(const vec2& start, const vec2& end) : start(start), end(end) {}
	vec2 start;
	vec2 end;
};

double distance(const vec2& a, const vec2& b);
bool intersects(const line& a, const line& b);

}
