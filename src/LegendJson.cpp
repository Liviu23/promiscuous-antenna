#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Errors.h"
#include "Legend.h"
#include "internal/JsonFile.h"

namespace {
	ConfigError invalidColor(const std::string& color) {
		return ConfigError{"culoare invalida: \"" + color + "\" (astept #rrggbb)"};
	}

	std::uint8_t hexByte(const std::string& color, std::size_t at) {
		std::size_t used = 0;
		const int value = std::stoi(color.substr(at, 2), &used, 16);
		if (used != 2) throw invalidColor(color);
		return static_cast<std::uint8_t>(value);
	}

	Glyph parseGlyph(const nlohmann::json& node) {
		const auto color = node.at("color").get<std::string>();
		if (color.size() != 7 || color.front() != '#') throw invalidColor(color);
		try {
			return Glyph{node.at("symbol").get<std::string>(), hexByte(color, 1), hexByte(color, 3), hexByte(color, 5)};
		} catch (const std::invalid_argument&) {  // aruncat de std::stoi pentru cifre hexa invalide
			throw invalidColor(color);
		}
	}

	LayerProfile parseProfile(const nlohmann::json& node) {
		std::vector<Layer> order;
		for (const auto& name : node.at("order")) {
			const auto layer = layerFromName(name.get<std::string>());
			if (!layer) throw ConfigError{"strat necunoscut in profil: " + name.get<std::string>()};
			order.push_back(*layer);
		}
		return LayerProfile{node.at("name").get<std::string>(), std::move(order)};
	}
} // namespace

Legend Legend::loadFromFile(const std::string& path) {
	const nlohmann::json root = readJsonFile(path);
	try {
		std::map<std::string, Glyph> glyphs;
		for (const auto& [kind, glyph] : root.at("kinds").items()) glyphs.emplace(kind, parseGlyph(glyph));

		std::vector<LayerProfile> profiles;
		for (const auto& profile : root.at("profiles")) profiles.push_back(parseProfile(profile));

		return Legend{root.at("cellWidth").get<int>(), parseGlyph(root.at("empty")), parseGlyph(root.at("unknown")),
					  std::move(glyphs), std::move(profiles)};
	} catch (const nlohmann::json::exception& error) {
		throw ConfigError{path + ": " + error.what()};
	}
}
