#pragma once

#include <iosfwd>
#include <vector>

#include "Coord.h"
#include "TorusGeometry.h"

// Amprenta unei entități: deplasări față de o celulă-ancoră.
// Ancora (0, 0) face mereu parte din amprentă.
class Footprint {
public:
	// aruncă ConfigError dacă lista e goală, nu conține (0, 0) sau are duplicate
	explicit Footprint(std::vector<Coord> offsets);

	// Celulele ocupate dacă ancora e în `anchor`, deja cu wrap.
	// Aruncă ConfigError dacă amprenta e mai mare decât harta (s-ar suprapune cu ea însăși).
	[[nodiscard]] std::vector<Coord> cellsAt(const Coord& anchor, const TorusGeometry& geometry) const;

	[[nodiscard]] const std::vector<Coord>& offsets() const;

	friend std::ostream& operator<<(std::ostream& os, const Footprint& footprint);

private:
	static bool hasAnchor(const std::vector<Coord>& cells);
	static bool hasDuplicates(const std::vector<Coord>& cells);

	std::vector<Coord> offsets_;
};
