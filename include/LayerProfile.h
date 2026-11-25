#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "MapTypes.h"

// Primul strat ocupat decide ce se desenează, iar straturile care lipsesc din listă nu se văd deloc.
class LayerProfile {
public:
	LayerProfile(std::string name, std::vector<Layer> order);  // aruncă ConfigError dacă ordinea e goală

	[[nodiscard]] const std::string& name() const;
	[[nodiscard]] const std::vector<Layer>& order() const;

	friend std::ostream& operator<<(std::ostream& os, const LayerProfile& profile);

private:
	std::string name_;
	std::vector<Layer> order_;
};
