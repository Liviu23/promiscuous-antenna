#include "WorldMap.h"

#include <algorithm>
#include <ostream>
#include <utility>

#include "Errors.h"

WorldMap::WorldMap(const TorusGeometry& geometry, const LayerCapacity& capacity) :
	geometry_{geometry}, cells_(geometry.cellCount(), Cell{capacity}) {}

WorldMap::WorldMap(const WorldMap& other) = default;

WorldMap::WorldMap(WorldMap&& other) noexcept = default;

WorldMap& WorldMap::operator=(WorldMap other) { swap(other); return *this; }

WorldMap::~WorldMap() = default;

void WorldMap::swap(WorldMap& other) noexcept {
	std::swap(geometry_, other.geometry_);
	std::swap(cells_, other.cells_);
	std::swap(entities_, other.entities_);
}

const TorusGeometry& WorldMap::geometry() const { return geometry_; }

const Cell& WorldMap::cellAt(const Coord& coord) const { return cells_[geometry_.indexOf(coord)]; }

Cell& WorldMap::mutableCellAt(const Coord& coord) { return cells_[geometry_.indexOf(coord)]; }

const WorldMap::Placement& WorldMap::placementOf(EntityId id) const {
	const auto it = entities_.find(id);
	if (it == entities_.end()) throw UnknownEntityError{id};
	return it->second;
}

void WorldMap::requireRoom(const std::vector<Coord>& cells, Layer layer, EntityId self) const {
	for (const Coord& coord : cells) {
		const Cell& cell = cellAt(coord);
		if (!cell.isOccupiedBy(layer, self) && !cell.hasRoom(layer))
			throw CellFullError{coord, layer};
	}
}

void WorldMap::occupy(EntityId id, Layer layer, const std::vector<Coord>& cells) {
	for (const Coord& coord : cells) mutableCellAt(coord).add(id, layer);
}

void WorldMap::vacate(EntityId id, Layer layer, const std::vector<Coord>& cells) {
	for (const Coord& coord : cells) mutableCellAt(coord).remove(id, layer);
}

WorldMap::Placement WorldMap::constructPlacement(const Layer& layer, const Coord& anchor, const Footprint& footprint) {
	return Placement {
		.footprint = footprint,
		.anchor = anchor,
		.layer = layer,
	};
}

void WorldMap::place(EntityId id, Layer layer, const Coord& anchor, const Footprint& footprint) {
	if (entities_.contains(id)) throw DuplicateEntityError{id};
	const Coord wrapped = geometry_.wrap(anchor);
	const std::vector<Coord> cells = footprint.cellsAt(wrapped, geometry_);
	requireRoom(cells, layer, id);

	occupy(id, layer, cells);
	entities_.emplace(id, constructPlacement(layer, wrapped, footprint));
}

void WorldMap::moveTo(EntityId id, const Coord& anchor) {
	const Placement& placement = placementOf(id);
	const Coord target = geometry_.wrap(anchor);
	const std::vector<Coord> oldCells = placement.footprint.cellsAt(placement.anchor, geometry_);
	const std::vector<Coord> newCells = placement.footprint.cellsAt(target, geometry_);
	requireRoom(newCells, placement.layer, id);

	vacate(id, placement.layer, oldCells);
	occupy(id, placement.layer, newCells);
	entities_.at(id).anchor = target;
}

void WorldMap::step(EntityId id, Direction direction) { moveTo(id, geometry_.step(anchorOf(id), direction)); }

void WorldMap::remove(EntityId id) {
	const Placement& placement = placementOf(id);
	vacate(id, placement.layer, placement.footprint.cellsAt(placement.anchor, geometry_));
	entities_.erase(id);
}

Coord WorldMap::anchorOf(EntityId id) const { return placementOf(id).anchor; }

std::vector<EntityId> WorldMap::entitiesInRange(const Coord& center, int radius, Metric metric) const {
	std::vector<EntityId> found;
	for (const Coord& coord : geometry_.area(center, radius, metric))
		for (const Layer layer : {Layer::Ground, Layer::Object, Layer::Animal}) {
			const auto& ids = cellAt(coord).occupants(layer);
			found.insert(found.end(), ids.begin(), ids.end());
		}

	// o entitate pe mai multe celule apare o singură dată
	std::ranges::sort(found);
	const auto duplicates = std::ranges::unique(found);
	found.erase(duplicates.begin(), duplicates.end());
	return found;
}

std::vector<Coord> WorldMap::freeCellsInRange(const Coord& center, int radius, Metric metric, Layer layer) const {
	std::vector<Coord> free;
	for (const Coord& coord : geometry_.area(center, radius, metric))
		if (cellAt(coord).hasRoom(layer)) free.push_back(coord);
	return free;
}

std::ostream& operator<<(std::ostream& os, const WorldMap& map) {
	os << "Harta: " << map.geometry_ << ", " << map.entities_.size() << " entitati\n";

	// unordered_map nu are ordine; sortăm id-urile ca afișarea să fie deterministă
	std::vector<EntityId> ids;
	ids.reserve(map.entities_.size());
	for (const auto& [id, placement] : map.entities_) {
		ids.push_back(id);
	}
	std::ranges::sort(ids);

	for (const EntityId id : ids) {
		const auto& placement = map.entities_.at(id);
		os << " " << id << " pe stratul " << placement.layer << " ancora " << placement.anchor << " amprenta "
		   << placement.footprint << '\n';
	}
	return os;
}
