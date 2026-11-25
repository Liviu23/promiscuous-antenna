#include "Cell.h"
#include "MapTypes.h"

#include <algorithm>
#include <ostream>
#include <stdexcept>

Cell::Cell(const LayerCapacity& capacity) : capacity_{capacity} {}

std::size_t Cell::asIndex(Layer layer) { return static_cast<std::size_t>(layer); }

const std::vector<EntityId>& Cell::occupants(Layer layer) const { return occupants_[asIndex(layer)]; }

std::size_t Cell::capacity(Layer layer) const { return capacity_[asIndex(layer)]; }

bool Cell::isOccupiedBy(Layer layer, EntityId id) const {
	return std::ranges::find(occupants(layer), id) != occupants(layer).end();
}

bool Cell::hasRoom(Layer layer) const { return occupants(layer).size() < capacity(layer); }

void Cell::add(EntityId id, Layer layer) {
	if (!hasRoom(layer)) throw std::logic_error{"Cell::add: stratul e plin, apelantul trebuia sa verifice hasRoom()"};
	occupants_[asIndex(layer)].push_back(id);
}

void Cell::remove(EntityId id, Layer layer) { std::erase(occupants_[asIndex(layer)], id); }

std::ostream& operator<<(std::ostream& os, const Cell& cell) {
	std::string padding[2] = { "", " " };

	for (const Layer layer : {Layer::Ground, Layer::Object, Layer::Animal}) {
		const auto& ids = cell.occupants(layer);
		os << "stratul " << layer << " are utilizarea " << ids.size() << '/' << cell.capacity(layer) << " [";
		for (std::size_t i = 0, pad = 0; i < ids.size(); ++i, pad = 1)
			os << padding[pad] << ids[i];
		if(ids.size() == 0) os << "nicio entitate";
		os << "]"  << padding[(layer != Layer::Animal)];
	}
	return os;
}
