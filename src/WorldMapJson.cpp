#include <algorithm>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "Errors.h"
#include "WorldMap.h"
#include "internal/JsonFile.h"

namespace {
	Layer parseLayer(const std::string& name) {
		const auto layer = layerFromName(name);
		if (!layer) throw ConfigError{"strat necunoscut: \"" + name + "\" (astept ground / object / animal)"};
		return *layer;
	}

	LayerCapacity parseCapacity(const nlohmann::json& node) {
		return {
			node.at("ground").get<std::size_t>(),
			node.at("object").get<std::size_t>(),
			node.at("animal").get<std::size_t>()
		};
	}

	Footprint parseFootprint(const nlohmann::json& node) {
		std::vector<Coord> offsets;
		for (const auto& pair : node) {
			offsets.emplace_back(pair.at(0).get<int>(), pair.at(1).get<int>());
		}
		return Footprint{offsets};
	}

	// vezi EntityKinds.h
	class EntityKind {
	public:
		EntityKind(Layer layer, Footprint footprint) : layer_{layer}, footprint_{std::move(footprint)} {}
		[[nodiscard]] Layer layer() const { return layer_; }
		[[nodiscard]] const Footprint& footprint() const { return footprint_; }

	private:
		Layer layer_;
		Footprint footprint_;
	};

	std::map<std::string, EntityKind> parseKinds(const nlohmann::json& node) {
		std::map<std::string, EntityKind> kinds;
		for (const auto& [name, kind] : node.items()) {
			kinds.emplace(name, EntityKind{parseLayer(kind.at("layer").get<std::string>()),
										   parseFootprint(kind.at("footprint"))});
		}
		return kinds;
	}

	std::string layerName(Layer layer) {
		std::ostringstream text;
		text << layer;
		return text.str();
	}

	nlohmann::ordered_json footprintJson(const Footprint& footprint) {
		auto offsets = nlohmann::ordered_json::array();
		for (const Coord& offset : footprint.offsets()) offsets.push_back({offset.x(), offset.y()});
		return offsets;
	}

	// Câte un tip / o entitate pe rând, ca fișierul să arate ca assets/map.json și să fie ușor de citit.
	void writeLines(std::ostream& out, const std::vector<std::string>& lines) {
		for (std::size_t i = 0; i < lines.size(); ++i) out << "    " << lines[i] << (i + 1 < lines.size() ? ",\n" : "\n");
	}
} // namespace

WorldMap WorldMap::loadFromFile(const std::string& path) {
	const nlohmann::json root = readJsonFile(path);
	try {
		WorldMap map{
			TorusGeometry{
				root.at("width").get<int>(),
				root.at("height").get<int>()
			},
			parseCapacity(root.at("capacity"))
		};
		const auto kinds = parseKinds(root.at("kinds"));

		for (const auto& entity : root.at("entities")) {
			const auto typeName = entity.at("kind").get<std::string>();
			const auto kind = kinds.find(typeName);
			if (kind == kinds.end()) {
				throw ConfigError{"tip de entitate necunoscut: \"" + typeName + "\""};
			}
			map.place(entity.at("id").get<EntityId>(), kind->second.layer(),
					  Coord{entity.at("x").get<int>(), entity.at("y").get<int>()}, kind->second.footprint());
		}
		return map;
	} catch (const nlohmann::json::exception& error) {
		// câmp lipsă sau de tip greșit: îl traducem într-o eroare a proiectului
		throw ConfigError{path + ": " + error.what()};
	}
}

void WorldMap::saveToFile(const std::string& path, const EntityKinds& kinds) const {
	std::vector<EntityId> ids;
	ids.reserve(entities_.size());
	for (const auto& [id, placement] : entities_) ids.push_back(id);
	std::ranges::sort(ids);

	std::map<std::string, nlohmann::ordered_json> kindDefinitions;
	std::vector<std::string> entityLines;
	entityLines.reserve(ids.size());
	for (const EntityId id : ids) {
		const Placement& placement = entities_.at(id);
		const auto kind = kinds.kindOf(id);
		if (!kind) {
			std::ostringstream message;
			message << id << " nu are un tip in catalog";
			throw StorageError{message.str()};
		}
		kindDefinitions.try_emplace(*kind, nlohmann::ordered_json{
			{"layer", layerName(placement.layer)},
			{"footprint", footprintJson(placement.footprint)}
		});
		entityLines.push_back(nlohmann::ordered_json{
			{"id", id}, {"kind", *kind}, {"x", placement.anchor.x()}, {"y", placement.anchor.y()}
		}.dump());
	}

	std::vector<std::string> kindLines;
	kindLines.reserve(kindDefinitions.size());
	for (const auto& [name, definition] : kindDefinitions)
		kindLines.push_back(nlohmann::json(name).dump() + ": " + definition.dump());

	// toate celulele au aceeași capacitate, deci o citim din prima
	const Cell& anyCell = cells_.front();
	const nlohmann::ordered_json capacity{
		{"ground", anyCell.capacity(Layer::Ground)},
		{"object", anyCell.capacity(Layer::Object)},
		{"animal", anyCell.capacity(Layer::Animal)}
	};

	std::ostringstream text;
	text << "{\n";
	text << "  \"width\": " << geometry_.width() << ",\n";
	text << "  \"height\": " << geometry_.height() << ",\n";
	text << "  \"capacity\": " << capacity.dump() << ",\n";
	text << "  \"kinds\": {\n";
	writeLines(text, kindLines);
	text << "  },\n";
	text << "  \"entities\": [\n";
	writeLines(text, entityLines);
	text << "  ]\n}\n";

	std::ofstream file{path};
	if (!file) throw StorageError{"nu pot scrie fisierul " + path};
	file << text.str();
	if (!file) throw StorageError{"scrierea in " + path + " a esuat"};
}
