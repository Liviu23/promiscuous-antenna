#include "internal/JsonFile.h"

#include <fstream>

#include "Errors.h"

nlohmann::json readJsonFile(const std::string& path) {
	std::ifstream file{path};
	if (!file) throw ConfigError{"nu pot deschide fisierul " + path};
	try { return nlohmann::json::parse(file); }
	catch (const nlohmann::json::parse_error& error) { throw ConfigError{path + ": " + error.what()}; }
}
