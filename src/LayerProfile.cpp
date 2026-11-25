#include "LayerProfile.h"

#include <ostream>
#include <utility>

#include "Errors.h"

LayerProfile::LayerProfile(std::string name, std::vector<Layer> order) : name_{std::move(name)}, order_{std::move(order)} {
	if (order_.empty()) throw ConfigError{"profilul \"" + name_ + "\" nu afiseaza niciun strat"};
}

const std::string& LayerProfile::name() const { return name_; }
const std::vector<Layer>& LayerProfile::order() const { return order_; }

std::ostream& operator<<(std::ostream& os, const LayerProfile& profile) {
	std::string separators[2] = { "", " > "};
	os << profile.name_ << " [";
	for (std::size_t i = 0, sep = 0; i < profile.order_.size(); ++i, sep = 1)
		os << separators[sep] << profile.order_[i];
	return os << ']';
}
