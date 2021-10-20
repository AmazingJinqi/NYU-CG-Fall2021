////////////////////////////////////////////////////////////////////////////////
#include <algorithm>
#include <complex>
#include <fstream>
#include <iostream>
#include <numeric>
#include <vector>
////////////////////////////////////////////////////////////////////////////////

typedef std::complex<double> Point;
typedef std::vector<Point> Polygon;

double inline det(const Point &u, const Point &v) {
	// TODO
	return u.real() * v.imag() - u.imag() * v.real();
}

struct Compare {
	Point p0; // Leftmost point of the poly
	bool operator ()(const Point &p1, const Point &p2) {
		// TODO
		double value = det(p1 - p0, p2 - p0);
		if (value == 0) return p1.real() < p2.real();
		else return value > 0;
	}
};

bool inline salientAngle(Point &a, Point &b, Point &c) {
	// TODO
	return det(b - a, c - b) > 0;
}

////////////////////////////////////////////////////////////////////////////////

Polygon convex_hull(std::vector<Point> &points) {
	Compare order;
	// TODO
	order.p0 = Point(INT_MAX, INT_MAX);
	for (Point point : points){
		if (point.imag() < order.p0.imag() || (point.imag() == order.p0.imag() && point.real() < order.p0.real()))
			order.p0 = point;
	}
	std::sort(points.begin(), points.end(), order);
	Polygon hull;
	// TODO
	// use salientAngle(a, b, c) here
	for (Point point : points){
		while(hull.size() >= 2 && !salientAngle(hull[hull.size() - 2], hull[hull.size() - 1], point))
			hull.pop_back();
		hull.push_back(point);
	}
	return hull;
}

////////////////////////////////////////////////////////////////////////////////

std::vector<Point> load_xyz(const std::string &filename) {
	std::vector<Point> points;
	std::ifstream in(filename);
	// TODO
	if (!in.is_open()) {
		throw std::runtime_error("failed to open file " + filename);
	}
	int n;	// the number of points
	in >> n;
	for (int i = 0; i < n; i++)
	{
		double x, y, z;
		in >> x >> y >> z;
		points.push_back(Point(x, y));
	}
	return points;
}

void save_obj(const std::string &filename, Polygon &poly) {
	std::ofstream out(filename);
	if (!out.is_open()) {
		throw std::runtime_error("failed to open file " + filename);
	}
	out << std::fixed;
	for (const auto &v : poly) {
		out << "v " << v.real() << ' ' << v.imag() << " 0\n";
	}
	for (size_t i = 0; i < poly.size(); ++i) {
		out << "l " << i+1 << ' ' << 1+(i+1)%poly.size() << "\n";
	}
	out << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[]) {
	if (argc <= 2) {
		std::cerr << "Usage: " << argv[0] << " points.xyz output.obj" << std::endl;
	}
	std::vector<Point> points = load_xyz(argv[1]);
	Polygon hull = convex_hull(points);
	save_obj(argv[2], hull);
	std::cout << "yes";
	return 0;
}
