// assert() dispare cu NDEBUG (Release)
#undef NDEBUG
#include "MapTypes.h"
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <set>
#include <sstream>
#include <vector>

#include "Errors.h"
#include "Footprint.h"
#include "TorusGeometry.h"
#include "EntityKinds.h"
#include "Legend.h"
#include "Viewport.h"
#include "WorldMap.h"

namespace {
	template <typename Error, typename Action>
	bool throws(Action action) {
		try {
			action();
		} catch (const Error&) {
			return true;
		}
		return false;
	}

	bool contains(const std::vector<Coord>& cells, const Coord& wanted) {
		return std::ranges::find(cells, wanted) != cells.end();
	}

	std::size_t distinct(const std::vector<Coord>& cells) {
		std::set<std::pair<int, int>> unique;
		for (const Coord& cell : cells) {
			unique.emplace(cell.x(), cell.y());
		}
		return unique.size();
	}

	constexpr LayerCapacity smallCapacity{1, 1, 2};  // sol 1, obiect 1, animale 2

	void testWrapAndDistance() {
		const TorusGeometry geo{10, 6};
		assert((geo.wrap({-1, -1}) == Coord{9, 5}));
		assert((geo.wrap({10, 6}) == Coord{0, 0}));
		assert((geo.wrap({-21, 13}) == Coord{9, 1}));

		// peste margine: de la coloana 9 la coloana 0 e un singur pas spre dreapta
		assert((geo.shortestDelta({9, 0}, {0, 0}) == Coord{1, 0}));
		assert((geo.shortestDelta({0, 0}, {9, 0}) == Coord{-1, 0}));
		assert(geo.distance({9, 5}, {0, 0}, Metric::Manhattan) == 2);
		assert(geo.distance({9, 5}, {0, 0}, Metric::Chebyshev) == 1);
		// distanța maximă pe o axă e jumătate din latură
		assert(geo.distance({0, 0}, {5, 3}, Metric::Chebyshev) == 5);

		assert((geo.step({0, 0}, Direction::NW) == Coord{9, 5}));
		assert(geo.indexOf({-1, 0}) == 9);
		assert(geo.cellCount() == 60);
	}

	void testNeighborsAndArea() {
		const TorusGeometry tiny{TorusGeometry::minSide, TorusGeometry::minSide};
		const auto moore = tiny.neighbors({0, 0}, Neighborhood::Moore);
		assert(moore.size() == 8 && distinct(moore) == 8);
		assert(!contains(moore, {0, 0}));
		assert(tiny.neighbors({0, 0}, Neighborhood::VonNeumann).size() == 4);

		const TorusGeometry geo{10, 6};
		assert(geo.area({0, 0}, 0, Metric::Chebyshev).size() == 1);
		assert(geo.area({0, 0}, 1, Metric::Chebyshev).size() == 9);
		assert(geo.area({0, 0}, 1, Metric::Manhattan).size() == 5);
		assert(contains(geo.area({0, 0}, 1, Metric::Manhattan), {9, 0}));

		// o rază uriașă acoperă harta exact o dată, inclusiv pe laturi pare
		const auto everything = geo.area({3, 3}, 100, Metric::Chebyshev);
		assert(everything.size() == geo.cellCount() && distinct(everything) == geo.cellCount());

		assert(throws<ConfigError>([] { (void) TorusGeometry{2, 10}; }));
	}

	void testFootprint() {
		const TorusGeometry geo{4, 4};
		const Footprint square{{{0, 0}, {1, 0}, {0, 1}, {1, 1}}};
		const auto cells = square.cellsAt({3, 3}, geo);
		assert(contains(cells, {3, 3}) && contains(cells, {0, 3}) && contains(cells, {3, 0}) && contains(cells, {0, 0}));

		assert(throws<ConfigError>([] { (void) Footprint{{}}; }));
		assert(throws<ConfigError>([] { (void) Footprint{{{1, 0}}}; }));
		assert(throws<ConfigError>([] { (void) Footprint{{{0, 0}, {1, 0}, {1, 0}}}; }));
		// pe un tor de 4, deplasarea 4 cade peste ancoră
		assert(throws<ConfigError>([&] { (void) Footprint{{{0, 0}, {4, 0}}}.cellsAt({0, 0}, geo); }));
	}

	void testCapacityAndAtomicity() {
		WorldMap map{TorusGeometry{5, 5}, smallCapacity};
		const Footprint one{{{0, 0}}};
		map.place(EntityId{1}, Layer::Animal, {2, 2}, one);
		map.place(EntityId{2}, Layer::Animal, {2, 2}, one);
		map.place(EntityId{3}, Layer::Object, {2, 2}, one);  // alt strat: coexistă cu animalele
		assert(!map.cellAt({2, 2}).hasRoom(Layer::Animal));

		assert(throws<CellFullError>([&] { map.place(EntityId{4}, Layer::Animal, {2, 2}, one); }));
		assert(throws<DuplicateEntityError>([&] { map.place(EntityId{1}, Layer::Animal, {0, 0}, one); }));

		map.place(EntityId{4}, Layer::Animal, {0, 0}, one);
		assert(throws<CellFullError>([&] { map.moveTo(EntityId{4}, {2, 2}); }));
		// mutarea eșuată nu a schimbat nimic
		assert((map.anchorOf(EntityId{4}) == Coord{0, 0}));
		assert(map.cellAt({0, 0}).occupants(Layer::Animal).size() == 1);
		assert(map.cellAt({2, 2}).occupants(Layer::Animal).size() == 2);

		map.remove(EntityId{1});
		map.moveTo(EntityId{4}, {7, 7});  // (7, 7) e (2, 2) după wrap
		assert((map.anchorOf(EntityId{4}) == Coord{2, 2}));
		assert(map.cellAt({0, 0}).occupants(Layer::Animal).empty());

		assert(throws<UnknownEntityError>([&] { map.remove(EntityId{1}); }));
		assert(throws<UnknownEntityError>([&] { map.step(EntityId{1}, Direction::N); }));
	}

	void testLargeFootprintMoves() {
		WorldMap map{TorusGeometry{6, 6}, smallCapacity};
		const Footprint square{{{0, 0}, {1, 0}, {0, 1}, {1, 1}}};
		map.place(EntityId{1}, Layer::Object, {0, 0}, square);
		// un pas spre dreapta se suprapune cu vechea poziție: nu trebuie să se "blocheze" singur
		map.step(EntityId{1}, Direction::E);
		assert((map.anchorOf(EntityId{1}) == Coord{1, 0}));
		assert(map.cellAt({0, 0}).occupants(Layer::Object).empty());
		assert(map.cellAt({2, 1}).occupants(Layer::Object).size() == 1);

		map.place(EntityId{2}, Layer::Object, {4, 0}, Footprint{{{0, 0}}});
		assert(throws<CellFullError>([&] { map.moveTo(EntityId{1}, {3, 0}); }));
		assert((map.anchorOf(EntityId{1}) == Coord{1, 0}));
	}

	void testQueries() {
		WorldMap map{TorusGeometry{8, 8}, smallCapacity};
		const Footprint one{{{0, 0}}};
		map.place(EntityId{1}, Layer::Object, {7, 7}, Footprint{{{0, 0}, {1, 0}, {0, 1}, {1, 1}}});  // peste colț
		map.place(EntityId{2}, Layer::Animal, {0, 0}, one);
		map.place(EntityId{3}, Layer::Animal, {4, 4}, one);

		// tufișul ocupă 4 celule din zonă, dar apare o singură dată
		const auto heard = map.entitiesInRange({0, 0}, 1, Metric::Chebyshev);
		assert((heard == std::vector<EntityId>{EntityId{1}, EntityId{2}}));

		const auto freeGround = map.freeCellsInRange({0, 0}, 1, Metric::Chebyshev, Layer::Ground);
		assert(freeGround.size() == 9);
		const auto freeObject = map.freeCellsInRange({0, 0}, 1, Metric::Chebyshev, Layer::Object);
		assert(freeObject.size() == 5 && !contains(freeObject, {7, 7}));
	}

	void testCopiesAreIndependent() {
		WorldMap original{TorusGeometry{5, 5}, smallCapacity};
		original.place(EntityId{1}, Layer::Animal, {1, 1}, Footprint{{{0, 0}}});

		WorldMap copy{original};
		copy.step(EntityId{1}, Direction::S);
		assert((original.anchorOf(EntityId{1}) == Coord{1, 1}));
		assert((copy.anchorOf(EntityId{1}) == Coord{1, 2}));

		copy = original;
		assert((copy.anchorOf(EntityId{1}) == Coord{1, 1}));
		assert(copy.cellAt({1, 2}).occupants(Layer::Animal).empty());
	}

	void testViewport() {
		const TorusGeometry geo{10, 6};
		Viewport view{geo, 2};
		// la început: toată harta, o singură dată
		assert(view.columns() == 10 && view.rows() == 6);
		assert((view.worldAt(0, 0) == Coord{0, 0}));
		assert((view.center() == Coord{5, 3}));

		// fereastra peste margine: coloana 0 a ferestrei e ultima coloană a hărții
		view.pan(Direction::W);
		assert((view.worldAt(0, 0) == Coord{9, 0}));
		assert((view.worldAt(1, 0) == Coord{0, 0}));

		// o fereastră mai mare decât harta o repetă
		view.fitTo(2 * 15, 8);  // 15 celule de câte 2 coloane
		assert(view.columns() == 15 && view.rows() == 8);
		assert((view.worldAt(0, 0) == view.worldAt(10, 6)));

		// fitTo păstrează centrul; centerOn îl mută
		const Coord before = view.center();
		view.fitTo(2 * 7, 5);
		assert((view.center() == before));
		view.centerOn({0, 0});
		assert((view.center() == Coord{0, 0}));

		assert(throws<ConfigError>([&] { (void) Viewport{geo, 0}; }));
	}

	Legend testLegend() {
		const std::map<std::string, Glyph> glyphs{
			{"rabbit", Glyph{"r", 255, 255, 255}},
			{"bush", Glyph{"#", 0, 128, 0}},
		};
		return Legend{2, Glyph{".", 50, 50, 50}, Glyph{"?", 255, 0, 255}, glyphs,
					  {LayerProfile{"normal", {Layer::Animal, Layer::Object, Layer::Ground}},
					   LayerProfile{"ascuns", {Layer::Object, Layer::Animal}}}};
	}

	void testLegendLook() {
		const Legend legend = testLegend();
		EntityKinds kinds;
		kinds.assign(EntityId{1}, "bush");
		kinds.assign(EntityId{2}, "rabbit");
		kinds.assign(EntityId{3}, "rabbit");
		kinds.assign(EntityId{4}, "dragon");  // tip fără simbol în legendă
		assert(throws<DuplicateEntityError>([&] { kinds.assign(EntityId{1}, "rabbit"); }));
		assert(!kinds.kindOf(EntityId{99}));

		WorldMap map{TorusGeometry{4, 4}, LayerCapacity{1, 1, 3}};
		const Footprint one{{{0, 0}}};
		map.place(EntityId{1}, Layer::Object, {1, 1}, one);
		map.place(EntityId{2}, Layer::Animal, {1, 1}, one);
		map.place(EntityId{3}, Layer::Animal, {1, 1}, one);
		map.place(EntityId{4}, Layer::Animal, {2, 2}, one);
		map.place(EntityId{5}, Layer::Animal, {3, 3}, one);  // lipsește din catalog

		const LayerProfile& normal = legend.profile(0);
		const LayerProfile& hidden = legend.profile(1);
		// textul are mereu cellWidth coloane: simbol + câți sunt pe strat
		assert(legend.look(map.cellAt({1, 1}), kinds, normal).text() == "r2");
		assert(legend.look(map.cellAt({1, 1}), kinds, hidden).text() == "# ");
		assert(legend.look(map.cellAt({0, 0}), kinds, normal).text() == ". ");
		assert(legend.look(map.cellAt({2, 2}), kinds, normal).text() == "? ");
		assert(legend.look(map.cellAt({3, 3}), kinds, normal).text() == "? ");
		assert(legend.look(map.cellAt({1, 1}), kinds, normal).green() == 255);

		assert(legend.profileCount() == 2);
		assert(&legend.profile(2) == &legend.profile(0));  // indexul trece prin modulo
		assert(throws<ConfigError>([] { (void) LayerProfile{"gol", {}}; }));
	}

	std::string readAll(const std::filesystem::path& path) {
		std::ifstream file{path};
		std::ostringstream text;
		text << file.rdbuf();
		return text.str();
	}

	void testSaveAndLoad() {
		WorldMap map{TorusGeometry{6, 5}, LayerCapacity{1, 1, 2}};
		EntityKinds kinds;
		map.place(EntityId{1}, Layer::Object, {5, 4}, Footprint{{{0, 0}, {1, 0}, {0, 1}, {1, 1}}});
		map.place(EntityId{2}, Layer::Animal, {0, 0}, Footprint{{{0, 0}}});
		map.place(EntityId{3}, Layer::Animal, {0, 0}, Footprint{{{0, 0}}});
		kinds.assign(EntityId{1}, "bush");
		kinds.assign(EntityId{2}, "rabbit");
		kinds.assign(EntityId{3}, "rabbit");

		const auto path = std::filesystem::temp_directory_path() / "promiscuous_antenna_test_save.json";
		map.saveToFile(path.string(), kinds);

		const WorldMap loaded = WorldMap::loadFromFile(path.string());
		const EntityKinds loadedKinds = EntityKinds::loadFromFile(path.string());
		assert(loaded.geometry().width() == 6 && loaded.geometry().height() == 5);
		assert((loaded.anchorOf(EntityId{1}) == Coord{5, 4}));
		assert(loaded.cellAt({0, 0}).occupants(Layer::Object).size() == 1);  // tufișul trece peste colț
		assert(loaded.cellAt({0, 0}).occupants(Layer::Animal).size() == 2);
		assert(!loaded.cellAt({0, 0}).hasRoom(Layer::Animal));               // capacitatea s-a păstrat
		assert(loadedKinds.kindOf(EntityId{3}) == "rabbit");

		// salvarea unei stări încărcate dă exact același fișier
		const auto again = std::filesystem::temp_directory_path() / "promiscuous_antenna_test_save_2.json";
		loaded.saveToFile(again.string(), loadedKinds);
		assert(readAll(path) == readAll(again));
		std::filesystem::remove(path);
		std::filesystem::remove(again);

		EntityKinds incomplete;
		incomplete.assign(EntityId{1}, "bush");
		const auto unused = std::filesystem::temp_directory_path() / "promiscuous_antenna_test_unused.json";
		assert(throws<StorageError>([&] { map.saveToFile(unused.string(), incomplete); }));
		const auto missingFolder = std::filesystem::temp_directory_path() / "promiscuous_antenna_nu_exista" / "x.json";
		assert(throws<StorageError>([&] { map.saveToFile(missingFolder.string(), kinds); }));
	}
} // namespace

int main() {
	testWrapAndDistance();
	testNeighborsAndArea();
	testFootprint();
	testCapacityAndAtomicity();
	testLargeFootprintMoves();
	testQueries();
	testCopiesAreIndependent();
	testViewport();
	testLegendLook();
	testSaveAndLoad();
	return 0;
}
