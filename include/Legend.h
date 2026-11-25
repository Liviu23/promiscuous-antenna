#pragma once

#include <cstddef>
#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include "Cell.h"
#include "EntityKinds.h"
#include "Glyph.h"
#include "LayerProfile.h"
#include "MapTypes.h"

// Legenda hărții: simbolul fiecărui tip, profilurile de afișare și lățimea unei celule.
class Legend {
public:
	Legend(int cellWidth, Glyph empty, Glyph unknown,
		std::map<std::string, Glyph> glyphs, std::vector<LayerProfile> profiles);  // aruncă ConfigError dacă nu există niciun profil

	static Legend loadFromFile(const std::string& path);

	[[nodiscard]] int cellWidth() const;
	[[nodiscard]] std::size_t profileCount() const;
	[[nodiscard]] const LayerProfile& profile(std::size_t index) const;

	// Ce se vede într-o celulă, cu textul completat exact la cellWidth coloane.
	// Simbolul primului ocupant de pe cel mai prioritar strat ocupat, plus câți sunt pe acel strat dacă sunt mai mulți.
	[[nodiscard]] Glyph look(const Cell& cell, const EntityKinds& kinds, const LayerProfile& profile) const;

	friend std::ostream& operator<<(std::ostream& os, const Legend& legend);

private:
	[[nodiscard]] const Glyph& glyphOf(EntityId id, const EntityKinds& kinds) const;
	[[nodiscard]] Glyph padded(const Glyph& glyph, std::size_t count) const;

	int cellWidth_;
	Glyph empty_;
	Glyph unknown_;
	std::map<std::string, Glyph> glyphs_;
	std::vector<LayerProfile> profiles_;
};
