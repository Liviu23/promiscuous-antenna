#include "Errors.h"
#include "MapTypes.h"

#include <sstream>
#include <string>

SimulationError::SimulationError(const std::string& message) : std::runtime_error{message} {}

ConfigError::ConfigError(const std::string& message) : SimulationError{"configurare invalida: " + message} {}

namespace {
	std::string cellFullMessage(const Coord& where, Layer layer) {
		std::ostringstream text;
		text << "celula " << where << " nu mai are loc pe stratul " << layer;
		return text.str();
	}

	std::string entityMessage(EntityId id, const std::string& msg) {
		std::ostringstream text;
		text << id << msg;
		return text.str();
	}
} // namespace

CellFullError::CellFullError(const Coord& where, Layer layer) : SimulationError{cellFullMessage(where, layer)} {}

UnknownEntityError::UnknownEntityError(EntityId id) : SimulationError{entityMessage(id, " nu este pe harta")} {}
DuplicateEntityError::DuplicateEntityError(EntityId id) : SimulationError{entityMessage(id, " este deja pe harta")} {}

StorageError::StorageError(const std::string& message) : SimulationError{"nu pot salva: " + message} {}
