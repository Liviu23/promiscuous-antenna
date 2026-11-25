#include "EntityKinds.h"

#include <algorithm>
#include <ostream>
#include <vector>

#include "Errors.h"
#include "internal/JsonFile.h"

EntityKinds EntityKinds::loadFromFile(const std::string& path) {
	const nlohmann::json root = readJsonFile(path);
	try {
		const auto& declared = root.at("kinds");
		EntityKinds kinds;
		for (const auto& entity : root.at("entities")) {
			const auto kind = entity.at("kind").get<std::string>();
			if (!declared.contains(kind)) throw ConfigError{"tip de entitate necunoscut: \"" + kind + "\""};
			kinds.assign(EntityId{entity.at("id").get<std::uint32_t>()}, kind);
		}
		return kinds;
	} catch (const nlohmann::json::exception& error) {
		throw ConfigError{path + ": " + error.what()};
	}
}

EntityKinds::EntityKinds() : kinds_{} {}
EntityKinds::EntityKinds(std::unordered_map<EntityId, std::string> kinds) : kinds_{std::move(kinds)} {}

void EntityKinds::assign(EntityId id, const std::string& kind) {
	if (!kinds_.emplace(id, kind).second) throw DuplicateEntityError{id};
}

std::optional<std::string> EntityKinds::kindOf(EntityId id) const {
	const auto it = kinds_.find(id);
	if (it == kinds_.end()) return std::nullopt;
	return it->second;
}

std::ostream& operator<<(std::ostream& os, const EntityKinds& kinds) {
	std::vector<EntityId> ids;
	ids.reserve(kinds.kinds_.size());
	for (const auto& [id, kind] : kinds.kinds_) ids.push_back(id);
	std::ranges::sort(ids);

	os << "Catalog: " << ids.size() << " entitati\n";
	for (const EntityId id : ids) os << "  " << id << ": " << kinds.kinds_.at(id) << '\n';
	return os;
}
