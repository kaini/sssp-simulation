#include "math.hpp"
#include <boost/qvm/all.hpp>
#include <cmath>
#include <cfloat>

double sssp::distance(const vec2& a, const vec2& b) {
	double dx = a.x - b.x;
	double dy = a.y - b.y;
	return std::sqrt(dx * dx + dy * dy);
}

bool sssp::intersects(const line& a, const line& b) {
	// First line:
	// x1 + lambda*s1
	// y1 + lambda*t2
	double x1 = a.start.x;
	double y1 = a.start.y;
	double s1 = a.end.x - a.start.x;
	double t1 = a.end.y - a.start.y;

	// Second line:
	// x2 + delta*s2
	// y2 + delta*t2
	double x2 = b.start.x;
	double y2 = b.start.y;
	double s2 = b.end.x - b.start.x;
	double t2 = b.end.y - b.start.y;

	// Equate the two equations to find delta.
	double quot = s1*t2 - s2*t1;
	if (std::abs(quot) < DBL_EPSILON) {
		// The lines are parallel.
		return false;
	}
	double delta = (t1 * (x2 - x1) + s1 * (y1 - y2)) / quot;
	if (delta < 0 || delta > 1) {
		// The lines intersect but not the finite segments.
		return false;
	}

	// Equate the two equations to find lambda.
	double lambda = (t2 * (x2 - x1) + s2 * (y1 - y2)) / quot;
	if (lambda < 0 || lambda > 1) {
		// The lines intersect but not the finite segments.
		return false;
	}

	// The lines intersect.
	return true;
}
