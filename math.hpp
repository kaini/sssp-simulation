#pragma once
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/segment.hpp>
#include <boost/optional.hpp>
#include <vector>

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

inline vec2 operator+(const vec2& a, const vec2& b) {
    return vec2(a.x + b.x, a.y + b.y);
}

inline vec2 operator-(const vec2& a, const vec2& b) {
    return vec2(a.x - b.x, a.y - b.y);
}

inline vec2 normalize(const vec2& v) {
    double f = std::sqrt(v.x * v.x + v.y * v.y);
    return vec2(v.x / f, v.y / f);
}

struct line {
    line(const vec2& start, const vec2& end) : start(start), end(end) {}
    vec2 start;
    vec2 end;
};

double distance(const vec2& a, const vec2& b);
bool intersects(const line& a, const line& b);

} // namespace sssp

BOOST_GEOMETRY_REGISTER_POINT_2D(sssp::vec2, double, boost::geometry::cs::cartesian, x, y)
BOOST_GEOMETRY_REGISTER_SEGMENT(sssp::line, sssp::vec2, start, end)
