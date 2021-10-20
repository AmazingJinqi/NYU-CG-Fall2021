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

// Return true iff [a,b] intersects [c,d], and store the intersection in ans
bool intersect_segment(const Point &a, const Point &b, const Point &c, const Point &d, Point &ans) {
	// TODO
	bool flag = det(b - a, c - a) * det(b - a, d - a) < 0 && det(d - c, a - c) * det(d - c, b - c) < 0;
	if (flag){
		ans = a + (b - a) * det(d - c, a - c) / det(b - a, d - c);
	}
	return flag;
}

////////////////////////////////////////////////////////////////////////////////

bool is_inside(const Polygon &poly, const Point &query) {
	// 1. Compute bounding box and set coordinate of a point outside the polygon
	// TODO
	double maxX = 0, maxY = 0;
	for (Point p : poly){
		maxX = maxX > p.real() ? maxX : p.real();
		maxY = maxY > p.imag() ? maxY : p.imag();
	}
	Point outside(maxX + 1, maxY + 1);
	// 2. Cast a ray from the query point to the 'outside' point, count number of intersections
	// TODO
	int num = 0;
	for (int i = 0; i < poly.size(); i++){
		Point a = poly[i], b = poly[(i + 1) % poly.size()];
		Point ans;
		if (intersect_segment(a, b, query, outside, ans)) num++;
	}
	return num & 1;
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

Polygon load_obj(const std::string &filename) {
	std::ifstream in(filename);
	// TODO
	if (!in.is_open()) {
		throw std::runtime_error("failed to open file " + filename);
	}
	std::vector<Point> temp;
	Polygon poly;
	while(!in.eof()){
		char type;
		in >> type;
		if (type == 'v'){
			double x, y, z;
			in >> x >> y >> z;
			temp.push_back(Point(x, y));
		}
		else if (type == 'f'){
			while(!in.eof() && in.peek() != '\n'){
				int index;
				in >> index;
				poly.push_back(temp[index - 1]);
			}
		}
	}
	return poly;
}

void save_xyz(const std::string &filename, const std::vector<Point> &points) {
	// TODO
	std::ofstream out(filename);
		if (!out.is_open()) {
		throw std::runtime_error("failed to open file " + filename);
	}
	out << std::fixed;
	out << points.size() << std::endl;
	for(Point point : points){
		out << point.real() << " " << point.imag() << " " << 0 << std::endl;
	}
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[]) {
	if (argc <= 3) {
		std::cerr << "Usage: " << argv[0] << " points.xyz poly.obj result.xyz" << std::endl;
	}
	std::vector<Point> points = load_xyz(argv[1]);
	Polygon poly = load_obj(argv[2]);
	std::vector<Point> result;
	for (size_t i = 0; i < points.size(); ++i) {
		if (is_inside(poly, points[i])) {
			result.push_back(points[i]);
		}
	}
	save_xyz(argv[3], result);
	return 0;
}
