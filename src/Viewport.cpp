#include "Viewport.h"

#include <algorithm>
#include <ostream>

#include "Errors.h"

Viewport::Viewport(const TorusGeometry& geometry, int cellWidth) :
	geometry_{geometry}, cellWidth_{cellWidth}, origin_{0, 0}, size_{geometry.sizeVector()} {
	if (cellWidth < 1) throw ConfigError{"o celula trebuie sa ocupe cel putin o coloana de terminal"};
}

int Viewport::columns() const { return size_.x(); }
int Viewport::rows() const { return size_.y(); }

Coord Viewport::center() const { return worldAt(size_ / 2); }

Coord Viewport::worldAt(int column, int row) const { return worldAt(Coord{column, row}); }
Coord Viewport::worldAt(Coord coord) const { return geometry_.wrap(origin_ + coord); }

void Viewport::pan(Direction direction) { origin_ = geometry_.step(origin_, direction); }
void Viewport::panMore(Coord direction) { origin_ = origin_ + direction; }

void Viewport::centerOn(const Coord& target) { origin_ = geometry_.wrap(target - (size_ / 2)); }

void Viewport::fitTo(int terminalColumns, int terminalRows) {
	const Coord keep = center();
	// cel puțin o celulă, chiar dacă terminalul e minuscul
	size_ = Coord{std::max(1, terminalColumns / cellWidth_), std::max(1, terminalRows)};
	centerOn(keep);
}

std::ostream& operator<<(std::ostream& os, const Viewport& viewport) {
	return os << "fereastra " << viewport.size_ << " celule de la " << viewport.origin_ << " (" << viewport.cellWidth_ << " coloane/celula)";
}
