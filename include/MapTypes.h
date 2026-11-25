#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string_view>

enum class EntityId : std::uint32_t {};

// Straturile unei celule. Fiecare strat are propria capacitate, deci un iepure
// poate sta în aceeași celulă cu un tufiș fără să se lovească de el.
enum class Layer { Ground, Object, Animal };
inline constexpr std::size_t layerCount = 3;

// Capacitatea fiecărui strat, indexată după Layer.
using LayerCapacity = std::array<std::size_t /* poate ca 8 bytes e prea mult */, layerCount>;

// Direcții pe grilă; y crește în jos (ca în terminal), deci N înseamnă y - 1.
enum class Direction { N, NE, E, SE, S, SW, W, NW };

// Vecinătatea von Neumann = 4 vecini (N, E, S, W); Moore = 8 vecini.
enum class Neighborhood { VonNeumann, Moore };

// Manhattan: |dx| + |dy| (zona de rază r e un romb).
// Chebyshev: max(|dx|, |dy|) (zona de rază r e un pătrat).
enum class Metric { Manhattan, Chebyshev };

std::optional<Layer> layerFromName(std::string_view name);
std::optional<Direction> directionFromName(std::string_view name);

std::istream& operator>>(std::istream& in, EntityId& id);
std::ostream& operator<<(std::ostream& os, EntityId id);
std::ostream& operator<<(std::ostream& os, Layer layer);
std::ostream& operator<<(std::ostream& os, Direction direction);
