#pragma once

#include <stdexcept>
#include <string>

#include "Coord.h"
#include "MapTypes.h"

// Rădăcina excepțiilor proiectului
class SimulationError : public std::runtime_error {
public:
	explicit SimulationError(const std::string& message);
};

// Date de configurare invalide: fișier lipsă, JSON greșit, hartă prea mică, amprentă invalidă.
class ConfigError : public SimulationError {
public:
	explicit ConfigError(const std::string& message);
};

class CellFullError : public SimulationError {
public:
	CellFullError(const Coord& where, Layer layer);
};

class UnknownEntityError : public SimulationError {
public:
	explicit UnknownEntityError(EntityId id);
};

class DuplicateEntityError : public SimulationError {
public:
	explicit DuplicateEntityError(EntityId id);
};

class StorageError : public SimulationError {
public:
	explicit StorageError(const std::string& message);
};
