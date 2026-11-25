#include "MapConsole.h"

#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>

#include "Errors.h"

namespace {
	template <typename T>
	T read(std::istringstream& args, const char* what) {
		T value{};
		if (!(args >> value)) throw std::invalid_argument{std::string{"lipseste sau e invalid argumentul: "} + what};
		return value;
	}

	Coord readCoord(std::istringstream& args) {
		const int x = read<int>(args, "x");
		const int y = read<int>(args, "y");
		return {x, y};
	}

	Layer readLayer(std::istringstream& args) {
		const auto name = read<std::string>(args, "strat");
		const auto layer = layerFromName(name);
		if (!layer) throw std::invalid_argument{"strat necunoscut: " + name + " (ground / object / animal)"};
		return *layer;
	}

	Direction readDirection(std::istringstream& args) {
		const auto name = read<std::string>(args, "directie");
		const auto direction = directionFromName(name);
		if (!direction) throw std::invalid_argument{"directie necunoscuta: " + name + " (N NE E SE S SW W NW)"};
		return *direction;
	}

	constexpr Metric listenMetric = Metric::Chebyshev;
} // namespace

MapConsole::MapConsole(WorldMap& map, EntityKinds& kinds, const Legend& legend) :
	map_{map}, kinds_{kinds}, legend_{legend}, view_{map, kinds, legend} {}

void MapConsole::run(std::istream& in, std::ostream& out) {
	help(out);
	std::string line;
	while (std::getline(in, line)) {
		std::istringstream args{line};
		std::string command;
		if (!(args >> command)) continue;  // linie goală

		out << "> " << line << '\n';
		try {
			if (!execute(command, args, out)) return;
		} catch (const CellFullError& error) {
			out << "  nu se poate: " << error.what() << '\n';
		} catch (const SimulationError& error) {
			out << "  eroare: " << error.what() << '\n';
		} catch (const std::invalid_argument& error) {
			out << "  comanda gresita: " << error.what() << '\n';
		}
	}
}

bool MapConsole::execute(const std::string& command, std::istringstream& args, std::ostream& out) {
	if (command == "stop") return false;

	if (command == "harta") out << map_;
	else if (command == "celula") showCell(args, out);
	else if (command == "muta") moveTo(args, out);
	else if (command == "pas") step(args, out);
	else if (command == "distanta") distance(args, out);
	else if (command == "vecini") neighbors(args, out);
	else if (command == "aude") listen(args, out);
	else if (command == "liber") freeCells(args, out);
	else if (command == "sterge") remove(args, out);
	else if (command == "salveaza") save(out);
	else if (command == "restaureaza") restore(out);
	else if (command == "exporta") exportTo(args, out);
	else if (command == "importa") importFrom(args, out);
	else if (command == "catalog") out << kinds_;
	else if (command == "legenda") out << legend_;
	else if (command == "arata") show(out);
	else if (command == "profil") switchProfile(out);
	else if (command == "centreaza") center(args, out);
	else if (command == "fereastra") resizeView(args, out);
	else if (command == "interactiv") interactive(args, out);
	else help(out);

	return true;
}

void MapConsole::showCell(std::istringstream& args, std::ostream& out) const {
	const Coord where = map_.geometry().wrap(readCoord(args));
	out << "  " << where << ": " << map_.cellAt(where) << '\n';
}

void MapConsole::moveTo(std::istringstream& args, std::ostream& out) {
	const auto id = read<EntityId>(args, "id");
	map_.moveTo(id, readCoord(args));
	out << "  " << id << " e acum la " << map_.anchorOf(id) << '\n';
}

void MapConsole::step(std::istringstream& args, std::ostream& out) {
	const auto id = read<EntityId>(args, "id");
	const Direction direction = readDirection(args);
	map_.step(id, direction);
	out << "  " << id << " a facut un pas spre " << direction << ", e la " << map_.anchorOf(id) << '\n';
}

void MapConsole::distance(std::istringstream& args, std::ostream& out) const {
	const Coord from = readCoord(args);
	const Coord to = readCoord(args);
	const TorusGeometry& geometry = map_.geometry();
	out << "  " << from << " -> " << to << ": deplasare " << geometry.shortestDelta(from, to)
		<< ", manhattan " << geometry.distance(from, to, Metric::Manhattan)
		<< ", chebyshev " << geometry.distance(from, to, Metric::Chebyshev) << '\n';
}

void MapConsole::neighbors(std::istringstream& args, std::ostream& out) const {
	const Coord around = readCoord(args);
	const auto count = read<int>(args, "4 sau 8");
	if (count != 4 && count != 8) throw std::invalid_argument{"vecinatatea e 4 sau 8"};

	const Neighborhood kind = count == 4 ? Neighborhood::VonNeumann : Neighborhood::Moore;
	out << "  vecinii lui " << around << ':';
	for (const Coord& neighbor : map_.geometry().neighbors(around, kind))
		out << ' ' << neighbor;
	out << '\n';
}

void MapConsole::listen(std::istringstream& args, std::ostream& out) const {
	const auto id = read<EntityId>(args, "id");
	const auto radius = read<int>(args, "raza");
	out << "  " << id << " aude pe raza " << radius << ':';
	for (const EntityId other : map_.entitiesInRange(map_.anchorOf(id), radius, listenMetric))
		if (other != id) out << " " << other;
	out << '\n';
}

void MapConsole::freeCells(std::istringstream& args, std::ostream& out) const {
	const Coord around = readCoord(args);
	const auto radius = read<int>(args, "raza");
	const Layer layer = readLayer(args);
	out << "  loc pe stratul " << layer << " in jurul lui " << around << ':';
	for (const Coord& free : map_.freeCellsInRange(around, radius, listenMetric, layer))
		out << ' ' << free;
	out << '\n';
}

void MapConsole::remove(std::istringstream& args, std::ostream& out) {
	const auto id = read<EntityId>(args, "id");
	map_.remove(id);
	out << "  " << id << " a fost scoasa de pe harta\n";
}

void MapConsole::save(std::ostream& out) {
	saved_.emplace(map_);
	out << "  starea hartii a fost salvata\n";
}

void MapConsole::restore(std::ostream& out) {
	if (!saved_) {
		out << "  nu exista o stare salvata\n";
		return;
	}
	map_ = *saved_;
	out << "  harta a revenit la starea salvata\n";
}

void MapConsole::exportTo(std::istringstream& args, std::ostream& out) const {
	const auto path = read<std::string>(args, "fisier");
	map_.saveToFile(path, kinds_);
	out << "  starea a fost scrisa in " << path << '\n';
}

void MapConsole::importFrom(std::istringstream& args, std::ostream& out) {
	const auto path = read<std::string>(args, "fisier");
	// citim tot înainte să schimbăm ceva: dacă fișierul e greșit, rămâne starea veche
	WorldMap loaded = WorldMap::loadFromFile(path);
	EntityKinds loadedKinds = EntityKinds::loadFromFile(path);
	map_ = std::move(loaded);
	kinds_ = std::move(loadedKinds);
	view_.resetViewport();  // harta încărcată poate avea alte dimensiuni
	out << "  starea a fost incarcata din " << path << '\n';
}

void MapConsole::show(std::ostream& out) const { out << view_.snapshot() << '\n'; }

void MapConsole::switchProfile(std::ostream& out) {
	view_.nextProfile();
	out << "  profil de afisare: " << view_.profile() << '\n';
}

void MapConsole::center(std::istringstream& args, std::ostream& out) {
	view_.centerOn(readCoord(args));
	out << "  " << view_.viewport() << '\n';
}

void MapConsole::resizeView(std::istringstream& args, std::ostream& out) {
	const auto columns = read<int>(args, "coloane");
	const auto rows = read<int>(args, "randuri");
	view_.resize(columns, rows);
	out << "  " << view_.viewport() << '\n';
}

void MapConsole::interactive(std::istringstream& args, std::ostream& out) {
	int seconds = 0;
	args >> seconds;
	view_.interact(seconds);
	out << "  inapoi in consola\n";
}

void MapConsole::help(std::ostream& out) {
	out << "  comenzi: harta | celula x y | muta id x y | pas id DIR | distanta x1 y1 x2 y2 | vecini x y 4|8\n"
		   "           aude id raza | liber x y raza strat | sterge id | salveaza | restaureaza\n"
		   "           exporta fisier | importa fisier\n"
		   "           catalog | legenda | arata | profil | centreaza x y | fereastra coloane randuri\n"
		   "           interactiv [secunde] | stop\n";
}
