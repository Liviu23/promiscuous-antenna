#pragma once

#include <iosfwd>
#include <optional>
#include <sstream>
#include <string>

#include "EntityKinds.h"
#include "Legend.h"
#include "MapView.h"
#include "WorldMap.h"

// Consolă text pentru hartă: citește comenzi (din tastatura.txt în CI) și le aplică pe hartă.
// Afișarea grafică (ftxui) se deschide din consolă: "arata" pentru un cadru, "interactiv" pentru bucla cu tastatura.
// Comenzile sunt descrise în `help`.
class MapConsole {
public:
	MapConsole(WorldMap& map, EntityKinds& kinds, const Legend& legend);

	// Rulează până la "stop" sau sfârșitul intrării.
	void run(std::istream& in, std::ostream& out);

private:
	// Întoarce false când trebuie oprită consola.
	bool execute(const std::string& command, std::istringstream& args, std::ostream& out);

	void showCell(std::istringstream& args, std::ostream& out) const;
	void moveTo(std::istringstream& args, std::ostream& out);
	void step(std::istringstream& args, std::ostream& out);
	void distance(std::istringstream& args, std::ostream& out) const;
	void neighbors(std::istringstream& args, std::ostream& out) const;
	void listen(std::istringstream& args, std::ostream& out) const;
	void freeCells(std::istringstream& args, std::ostream& out) const;
	void remove(std::istringstream& args, std::ostream& out);
	void save(std::ostream& out);
	void restore(std::ostream& out);
	void exportTo(std::istringstream& args, std::ostream& out) const;
	void importFrom(std::istringstream& args, std::ostream& out);
	void show(std::ostream& out) const;
	void switchProfile(std::ostream& out);
	void center(std::istringstream& args, std::ostream& out);
	void resizeView(std::istringstream& args, std::ostream& out);
	void interactive(std::istringstream& args, std::ostream& out);
	static void help(std::ostream& out);

	WorldMap& map_;
	EntityKinds& kinds_;
	const Legend& legend_;
	std::optional<WorldMap> saved_;
	MapView view_;
};
