#pragma once

#include <iosfwd>
#include <string>
#include <unordered_map>
#include <vector>

#include "Cell.h"
#include "Coord.h"
#include "EntityKinds.h"
#include "Footprint.h"
#include "MapTypes.h"
#include "TorusGeometry.h"

// Harta lumii este singura sursă de adevăr pentru unde se află fiecare entitate.
// Entitățile sunt doar id-uri aici (în stil ECS) si toate operațiile care pot
// eșua sunt atomice: dacă aruncă erori, harta rămâne neschimbată.
class WorldMap {
public:
	WorldMap(const TorusGeometry& geometry, const LayerCapacity& capacity);

	// Citește dimensiunile, capacitățile, tipurile și plasările inițiale dintr-un fișier JSON.
	// Aruncă ConfigError (și erorile lui place()) dacă fișierul nu e valid.
	static WorldMap loadFromFile(const std::string& path);

	// Scrie starea în același format pe care îl citește loadFromFile, ca o salvare să poată fi încărcată înapoi.
	// Tipurile ("kinds") se reconstruiesc din entitățile de pe hartă. Aruncă StorageError.
	void saveToFile(const std::string& path, const EntityKinds& kinds) const;

	WorldMap(const WorldMap& other);
	WorldMap(WorldMap&& other) noexcept;
	WorldMap& operator=(WorldMap other);  // copy-and-swap: acoperă și copierea, și mutarea
	~WorldMap();

	[[nodiscard]] const TorusGeometry& geometry() const;
	[[nodiscard]] const Cell& cellAt(const Coord& coord) const;  // coordonata trece prin wrap

	void place(EntityId id, Layer layer, const Coord& anchor, const Footprint& footprint);
	void moveTo(EntityId id, const Coord& anchor);
	void step(EntityId id, Direction direction);
	void remove(EntityId id);

	[[nodiscard]] Coord anchorOf(EntityId id) const;

	// Cine e la distanță <= radius de centru, pe orice strat, fără duplicate (sortat după id).
	// Pentru broadcast: un receiver întreabă harta cine îl poate auzi / pe cine poate auzi.
	[[nodiscard]] std::vector<EntityId> entitiesInRange(const Coord& center, int radius, Metric metric) const;

	// Celulele din zonă care mai au loc pe stratul cerut (ex.: unde poate crește iarbă nouă).
	[[nodiscard]] std::vector<Coord> freeCellsInRange(const Coord& center, int radius, Metric metric, Layer layer) const;

	friend std::ostream& operator<<(std::ostream& os, const WorldMap& map);

private:
	// Ce știe harta despre o entitate
	struct Placement {
		Footprint footprint;
		Coord anchor;
		Layer layer;
	};
	[[nodiscard]] static Placement constructPlacement(const Layer& layer, const Coord& anchor, const Footprint& footprint);

	void swap(WorldMap& other) noexcept;

	[[nodiscard]] const Placement& placementOf(EntityId id) const;  // aruncă UnknownEntityError
	[[nodiscard]] Cell& mutableCellAt(const Coord& coord);  // nume diferit: altfel ar ascunde varianta publică const

	// Aruncă CellFullError pentru prima celulă fără loc (locul ocupat deja de `self` nu contează).
	void requireRoom(const std::vector<Coord>& cells, Layer layer, EntityId self) const;
	void occupy(EntityId id, Layer layer, const std::vector<Coord>& cells);
	void vacate(EntityId id, Layer layer, const std::vector<Coord>& cells);

	TorusGeometry geometry_;
	std::vector<Cell> cells_;
	std::unordered_map<EntityId, Placement> entities_;
};
