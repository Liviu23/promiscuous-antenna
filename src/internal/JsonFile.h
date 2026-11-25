#pragma once

#include <string>

#include <json/json.hpp>

nlohmann::json readJsonFile(const std::string& path);
