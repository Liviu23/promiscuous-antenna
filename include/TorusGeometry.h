#pragma once

#include <cstddef>
#include <iosfwd>
#include <vector>

#include "Coord.h"
#include "MapTypes.h"

// Geometria unei grile închise pe tor: ieși prin dreapta, intri prin stânga; la fel sus/jos.
// Toate funcțiile acceptă coordonate "în afara" hărții și le aduc înapoi (wrap).
class TorusGeometry {
public:
	// Sub 3 celule pe o axă, vecinii nu mai sunt distincți (stânga == dreapta).
	static constexpr int minSide = 3;

	TorusGeometry(int width, int height);  // aruncă ConfigError dacă o latură < minSide

	[[nodiscard]] int width() const;
	[[nodiscard]] int height() const;
	[[nodiscard]] Coord sizeVector() const;
	[[nodiscard]] std::size_t cellCount() const;

	[[nodiscard]] Coord wrap(const Coord& coord) const;
	[[nodiscard]] Coord step(const Coord& from, Direction direction) const;

	// Deplasarea cea mai scurtă de la `from` la `to`, ținând cont că se poate trece peste margine.
	// Fiecare componentă e în (-latura/2, latura/2].
	[[nodiscard]] Coord shortestDelta(const Coord& from, const Coord& to) const;
	[[nodiscard]] int distance(const Coord& a, const Coord& b, Metric metric) const;

	[[nodiscard]] std::vector<Coord> neighbors(const Coord& center, Neighborhood neighborhood) const;

	// Toate celulele (distincte, deja cu wrap) aflate la distanță <= radius de centru.
	// O rază mai mare decât jumătate de hartă acoperă harta o singură dată, fără duplicate.
	[[nodiscard]] std::vector<Coord> area(const Coord& center, int radius, Metric metric) const;

	// Index liniar row-major (y * width + x), după wrap.
	[[nodiscard]] std::size_t indexOf(const Coord& coord) const;

	static Coord offsetOf(Direction direction);

private:
	static int wrapAxis(int value, int size);
	static int shortestAxis(int from, int to, int size);

	int width_;
	int height_;
};

std::ostream& operator<<(std::ostream& os, const TorusGeometry& geometry);
