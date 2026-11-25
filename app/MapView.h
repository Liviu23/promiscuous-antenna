#pragma once

#include <cstddef>
#include <string>

#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include "EntityKinds.h"
#include "Legend.h"
#include "Viewport.h"
#include "WorldMap.h"

class MapView {
public:
	MapView(const WorldMap& map, const EntityKinds& kinds, const Legend& legend);

	// Un cadru desenat ca text (cu culori ANSI)
	[[nodiscard]] std::string snapshot() const;

	// Bucla interactivă: săgeți = mută fereastra, Tab = profilul următor, q / Esc = înapoi în consolă.
	// Dacă seconds > 0, se închide singură după atâtea secunde (pentru rularea automată din CI).
	void interact(int seconds);

	void nextProfile();
	void centerOn(const Coord& target);
	void resize(int terminalColumns, int terminalRows);
	void resetViewport();  // după ce harta a fost înlocuită (ex. încărcată din fișier)

	[[nodiscard]] const LayerProfile& profile() const;
	[[nodiscard]] const Viewport& viewport() const;

private:
	[[nodiscard]] ftxui::Element frame() const;  // harta + bara de stare
	[[nodiscard]] ftxui::Element grid() const;
	[[nodiscard]] ftxui::Element statusBar() const;
	[[nodiscard]] std::string describe(const Cell& cell) const;  // conținutul unei celule, pe un rând
	bool handle(const ftxui::Event& event);

	static constexpr int statusRows = 2;

	const WorldMap& map_;
	const EntityKinds& kinds_;
	const Legend& legend_;
	Viewport viewport_;
	std::size_t profile_ = 0;
};
