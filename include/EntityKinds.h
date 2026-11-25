#pragma once

#include <iosfwd>
#include <optional>
#include <string>
#include <unordered_map>

#include "MapTypes.h"

// Componenta "tip" a entităților: id -> numele tipului din JSON ("rabbit", "bush", et cetera)
class EntityKinds {
public:
	// Citește perechile (id, kind) din lista "entities" a fișierului hărții.
	// Aruncă ConfigError pentru un tip care nu e declarat în "kinds".
	static EntityKinds loadFromFile(const std::string& path);
	explicit EntityKinds();
	explicit EntityKinds(std::unordered_map<EntityId, std::string> kinds);

	void assign(EntityId id, const std::string& kind);  // aruncă DuplicateEntityError

	// std::nullopt pentru un id necunoscut: afișarea îl desenează ca "necunoscut" în loc să se oprească.
	[[nodiscard]] std::optional<std::string> kindOf(EntityId id) const;

	friend std::ostream& operator<<(std::ostream& os, const EntityKinds& kinds);

private:
	std::unordered_map<EntityId, std::string> kinds_;
};
