#include "Legend.h"

#include <ostream>
#include <utility>

#include "Errors.h"

Legend::Legend(int cellWidth, Glyph empty, Glyph unknown, std::map<std::string, Glyph> glyphs, std::vector<LayerProfile> profiles) :
	cellWidth_{cellWidth}, empty_{std::move(empty)}, unknown_{std::move(unknown)}, glyphs_{std::move(glyphs)}, profiles_{std::move(profiles)} {
	if (cellWidth_ < 1) throw ConfigError{"cellWidth trebuie sa fie cel putin 1"};
	if (profiles_.empty()) throw ConfigError{"legenda trebuie sa aiba cel putin un profil de afisare"};
}

int Legend::cellWidth() const { return cellWidth_; }
std::size_t Legend::profileCount() const { return profiles_.size(); }
const LayerProfile& Legend::profile(std::size_t index) const { return profiles_[index % profiles_.size()]; }

const Glyph& Legend::glyphOf(EntityId id, const EntityKinds& kinds) const {
	const auto kind = kinds.kindOf(id);
	if (!kind) return unknown_;
	const auto it = glyphs_.find(*kind);
	return it == glyphs_.end() ? unknown_ : it->second;
}

Glyph Legend::padded(const Glyph& glyph, std::size_t count) const {
	// simbolul ocupă o coloană; restul de cellWidth - 1 coloane arată câte entități sunt (dacă e loc și sunt mai multe)
	const auto rest = static_cast<std::size_t>(cellWidth_ - 1);
	std::string suffix(rest, ' ');
	if (rest > 0 && count > 1) suffix.front() = count > 9 ? '+' : static_cast<char>('0' + count);
	return glyph.withText(glyph.text() + suffix);
}

Glyph Legend::look(const Cell& cell, const EntityKinds& kinds, const LayerProfile& profile) const {
	for (const Layer layer : profile.order()) {
		const auto& ids = cell.occupants(layer);
		// Aratam glyph-ul primului id chiar daca sunt tipuri diferite de entitati
		if (!ids.empty()) return padded(glyphOf(ids.front(), kinds), ids.size());
	}
	return padded(empty_, 0);
}

std::ostream& operator<<(std::ostream& os, const Legend& legend) {
	os << "Legenda: " << legend.cellWidth_ << " coloane/celula, gol " << legend.empty_ << ", necunoscut " << legend.unknown_ << '\n';
	for (const auto& [kind, glyph] : legend.glyphs_) os << "  " << kind << ": " << glyph << '\n';
	for (const auto& profile : legend.profiles_) os << "  profil " << profile << '\n';
	return os;
}
