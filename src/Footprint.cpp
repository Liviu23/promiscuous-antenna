#include "Footprint.h"

#include <algorithm>
#include <ostream>
#include <utility>

#include "Errors.h"

Footprint::Footprint(std::vector<Coord> offsets) : offsets_{std::move(offsets)} {
	if (offsets_.empty()) throw ConfigError{"amprenta nu poate fi goala"};
	if (!hasAnchor(offsets_)) throw ConfigError{"amprenta trebuie sa contina ancora (0, 0)"};
	if(hasDuplicates(offsets_)) throw ConfigError{"amprenta contine aceeasi deplasare de doua ori"};
}


bool Footprint::hasAnchor(const std::vector<Coord>& cells) {
	const auto not_found = cells.end();
	return std::ranges::find(cells, Coord{0, 0}) != not_found;
}

bool Footprint::hasDuplicates(const std::vector<Coord>& cells) {
	const auto not_found = cells.end();

	for (auto it = cells.begin(); it != cells.end(); ++it)
		if (std::find(std::next(it), cells.end(), *it) != not_found) return true;
	return false;
}

std::vector<Coord> Footprint::cellsAt(const Coord& anchor, const TorusGeometry& geometry) const {
	std::vector<Coord> cells(offsets_.size(), Coord{0, 0});
	for(std::size_t i = 0; i < offsets_.size(); i++) cells[i] = geometry.wrap(anchor + offsets_[i]);

	if (hasDuplicates(cells)) throw ConfigError{"amprenta nu incape pe harta"};
	return cells;
}

const std::vector<Coord>& Footprint::offsets() const { return offsets_; }

std::ostream& operator<<(std::ostream& os, const Footprint& footprint) {
	std::string padding[2] = { "", " "};
	os << '{';
	for (std::size_t i = 0, pad = 0; i < footprint.offsets_.size(); ++i, pad = 1)
		os << padding[pad] << footprint.offsets_[i];
	return os << '}';
}
