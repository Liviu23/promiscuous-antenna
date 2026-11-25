#pragma once

#include <array>
#include <cstddef>
#include <iosfwd>
#include <vector>

#include "MapTypes.h"

// O celulă a hărții: pe fiecare strat ține id-urile entităților prezente, până la o capacitate.
// Celula nu își știe coordonatele; harta le știe.
class Cell {
public:
	explicit Cell(const LayerCapacity& capacity);

	[[nodiscard]] bool isOccupiedBy(Layer layer, EntityId id) const;
	[[nodiscard]] bool hasRoom(Layer layer) const;
	void add(EntityId id, Layer layer);     // precondiție hasRoom(layer), altfel std::logic_error
	void remove(EntityId id, Layer layer);  // fără efect dacă id-ul nu e acolo

	[[nodiscard]] const std::vector<EntityId>& occupants(Layer layer) const;
	[[nodiscard]] std::size_t capacity(Layer layer) const;

	friend std::ostream& operator<<(std::ostream& os, const Cell& cell);

private:
	static std::size_t asIndex(Layer layer);

	LayerCapacity capacity_;
	std::array<std::vector<EntityId>, layerCount> occupants_;
};
