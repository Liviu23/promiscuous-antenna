#include "TorusGeometry.h"

#include <algorithm>
#include <cstdlib>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>

#include "Coord.h"
#include "Errors.h"

namespace {
	// Intervalul de deplasări [lo, hi] pe o axă de lungime `size`, pentru o rază dată,
	// ales astfel încât nicio coloană/linie să nu fie vizitată de două ori după wrap.
	std::pair<int, int> axisRange(int radius, int size) {
		const int reach = std::min(radius, size / 2);
		// pe o latură pară, -size/2 și +size/2 sunt aceeași linie: o păstrăm o singură dată
		const int hi = (2 * reach + 1 > size) ? reach - 1 : reach;
		return {-reach, hi};
	}
} // namespace

TorusGeometry::TorusGeometry(int width, int height) : width_{width}, height_{height} {
	if (width < minSide || height < minSide) throw ConfigError {
		"harta trebuie sa aiba cel putin " +
		std::to_string(minSide) +
		" celule pe fiecare axa, are " +
		std::to_string(width) +
		"x" +
		std::to_string(height)
	};
}

int TorusGeometry::width() const { return width_; }

int TorusGeometry::height() const { return height_; }

Coord TorusGeometry::sizeVector() const { return Coord{width_, height_}; }

std::size_t TorusGeometry::cellCount() const { return static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_); }

int TorusGeometry::wrapAxis(int value, int size) { return ((value % size) + size) % size; }

int TorusGeometry::shortestAxis(int from, int to, int size) {
	int delta = wrapAxis(to - from, size);  // în [0, size)
	if (delta > size / 2) delta -= size;
	return delta;
}

Coord TorusGeometry::offsetOf(Direction direction) {
	switch (direction) {
		case Direction::N: return {0, -1};
		case Direction::NE: return {1, -1};
		case Direction::E: return {1, 0};
		case Direction::SE: return {1, 1};
		case Direction::S: return {0, 1};
		case Direction::SW: return {-1, 1};
		case Direction::W: return {-1, 0};
		case Direction::NW: return {-1, -1};
	}
	throw std::invalid_argument{"directie necunoscuta"};
}

Coord TorusGeometry::wrap(const Coord& coord) const {
	return {wrapAxis(coord.x(), width_), wrapAxis(coord.y(), height_)};
}

Coord TorusGeometry::step(const Coord& from, Direction direction) const {
	return wrap(from + offsetOf(direction));
}

Coord TorusGeometry::shortestDelta(const Coord& from, const Coord& to) const {
	return {shortestAxis(from.x(), to.x(), width_), shortestAxis(from.y(), to.y(), height_)};
}

int TorusGeometry::distance(const Coord& a, const Coord& b, Metric metric) const {
	const Coord delta = shortestDelta(a, b);
	const int dx = std::abs(delta.x());
	const int dy = std::abs(delta.y());
	switch (metric) {
		case Metric::Manhattan: return dx + dy;
		case Metric::Chebyshev: return std::max(dx, dy);
	}
	throw std::invalid_argument{"metrica necunoscuta"};
}

std::vector<Coord> TorusGeometry::neighbors(const Coord& center, Neighborhood neighborhood) const {
	static constexpr Direction cardinal[] = {Direction::N, Direction::E, Direction::S, Direction::W};
	static constexpr Direction all[] = {
		Direction::N, Direction::NE, Direction::E, Direction::SE,
		Direction::S, Direction::SW, Direction::W, Direction::NW
	};

	std::vector<Coord> result;
	if (neighborhood == Neighborhood::VonNeumann) {
		for (const Direction direction : cardinal) {
			result.push_back(step(center, direction));
		}
	} else {
		for (const Direction direction : all) {
			result.push_back(step(center, direction));
		}
	}
	return result;
}

std::vector<Coord> TorusGeometry::area(const Coord& center, int radius, Metric metric) const {
	if (radius < 0) {
		throw std::invalid_argument{"raza nu poate fi negativa"};
	}
	const auto [xLo, xHi] = axisRange(radius, width_);
	const auto [yLo, yHi] = axisRange(radius, height_);

	std::vector<Coord> result;
	for (int dy = yLo; dy <= yHi; ++dy) {
		for (int dx = xLo; dx <= xHi; ++dx) {
			// |dx| <= width/2 și |dy| <= height/2, deci (dx, dy) e chiar drumul cel mai scurt
			const int dist = (metric == Metric::Manhattan) ? std::abs(dx) + std::abs(dy)
														   : std::max(std::abs(dx), std::abs(dy));
			if (dist <= radius) {
				result.push_back(wrap(center + Coord{dx, dy}));
			}
		}
	}
	return result;
}

std::size_t TorusGeometry::indexOf(const Coord& coord) const {
	const Coord wrapped = wrap(coord);
	return static_cast<std::size_t>(wrapped.y()) * static_cast<std::size_t>(width_) +
		   static_cast<std::size_t>(wrapped.x());
}

std::ostream& operator<<(std::ostream& os, const TorusGeometry& geometry) {
	return os << "tor " << geometry.width() << 'x' << geometry.height();
}
