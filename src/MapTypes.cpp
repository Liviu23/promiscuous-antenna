#include "MapTypes.h"

#include <istream>
#include <algorithm>
#include <optional>
#include <ostream>
#include <utility>

namespace {
	constexpr std::array<std::pair<Layer, std::string_view>, layerCount> layerNames{{
		{Layer::Ground, "ground"},
		{Layer::Object, "object"},
		{Layer::Animal, "animal"},
	}};

	constexpr std::array<std::pair<Direction, std::string_view>, 8> directionNames{{
		{Direction::N, "N"},
		{Direction::NE, "NE"},
		{Direction::E, "E"},
		{Direction::SE, "SE"},
		{Direction::S, "S"},
		{Direction::SW, "SW"},
		{Direction::W, "W"},
		{Direction::NW, "NW"},
	}};

	template <typename Enum, std::size_t N>
	std::string_view nameOf(const std::array<std::pair<Enum, std::string_view>, N>& table, Enum value) {
		const auto it = std::ranges::find(table, value, &std::pair<Enum, std::string_view>::first);
		return it == table.end() ? "?" : it->second;
	}

	template <typename Enum, std::size_t N>
	std::optional<Enum> valueOf(const std::array<std::pair<Enum, std::string_view>, N>& table, std::string_view name) {
		const auto it = std::ranges::find(table, name, &std::pair<Enum, std::string_view>::second);
		return it == table.end() ? std::optional<Enum>(std::nullopt) : std::optional<Enum>(it->first);
	}
} // namespace

std::optional<Layer> layerFromName(std::string_view name) { return valueOf(layerNames, name); }
std::optional<Direction> directionFromName(std::string_view name) { return valueOf(directionNames, name); }

std::istream& operator>>(std::istream& in, EntityId& id) { std::uint32_t x; if(in >> x) id = EntityId{x}; return in; }
std::ostream& operator<<(std::ostream& os, EntityId id) { return os << "entitatea #" << std::to_underlying(id); }
std::ostream& operator<<(std::ostream& os, Layer layer) { return os << nameOf(layerNames, layer); }
std::ostream& operator<<(std::ostream& os, Direction direction) { return os << nameOf(directionNames, direction); }
