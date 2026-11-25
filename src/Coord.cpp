#include "Coord.h"

#include <ostream>

Coord::Coord(int x, int y) : x_{x}, y_{y} {}

int Coord::x() const { return x_; }
int Coord::y() const { return y_; }

Coord Coord::operator+(const Coord& other) const { return {x_ + other.x_, y_ + other.y_}; }
Coord Coord::operator-(const Coord& other) const { return {x_ - other.x_, y_ - other.y_}; }
Coord Coord::operator*(int s) const { return {x_ * s, y_ * s}; }
Coord Coord::operator/(int s) const { return {x_ / s, y_ / s}; }
bool Coord::operator==(const Coord& other) const = default;

std::ostream& operator<<(std::ostream& os, const Coord& coord) {
	return os << '(' << coord.x() << ", " << coord.y() << ')';
}
