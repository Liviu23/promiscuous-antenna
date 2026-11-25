#pragma once

#include <iosfwd>

// O pereche (x, y) pe grilă: x = coloana (spre dreapta), y = rândul (în jos). Echivalentul unui vector matematic.
class Coord {
public:
	Coord(int x, int y);

	[[nodiscard]] int x() const;
	[[nodiscard]] int y() const;

	[[nodiscard]] Coord operator+(const Coord& other) const;
	[[nodiscard]] Coord operator-(const Coord& other) const;
	[[nodiscard]] Coord operator*(int s) const;
	[[nodiscard]] Coord operator/(int s) const;
	bool operator==(const Coord& other) const;

private:
	int x_;
	int y_;
};

std::ostream& operator<<(std::ostream& os, const Coord& coord);
