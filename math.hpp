#pragma once

namespace sssp {

struct vec2 {
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
	vec2 start;
	vec2 end;
};

double distance(const vec2& a, const vec2& b);
bool intersects(const line& a, const line& b);

}
