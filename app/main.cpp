#include <iostream>
#include <optional>

#include "EntityKinds.h"
#include "Errors.h"
#include "Legend.h"
#include "MapConsole.h"
#include "WorldMap.h"

namespace {
	// copy_files(DIRECTORY assets ...) din CMakeLists.txt.
	constexpr const char* mapFile = "assets/map.json";
	constexpr const char* legendFile = "assets/legend.json";
} // namespace

int main() {
	std::optional<WorldMap> map;
	std::optional<EntityKinds> kinds;
	std::optional<Legend> legend;
	try {
		map.emplace(WorldMap::loadFromFile(mapFile));
		kinds.emplace(EntityKinds::loadFromFile(mapFile));
		legend.emplace(Legend::loadFromFile(legendFile));
	} catch (const SimulationError& error) {
		std::cerr << "Nu pot porni simularea: " << error.what() << '\n';
		return 1;
	}

	MapConsole console{*map, *kinds, *legend};
	console.run(std::cin, std::cout);
	return 0;
}
