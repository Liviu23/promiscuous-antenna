#pragma once

#include <iosfwd>

#include "Coord.h"
#include "MapTypes.h"
#include "TorusGeometry.h"

// O celulă de hartă ocupă `cellWidth` coloane de terminal (un caracter e cam de 2 ori mai înalt decât lat).
// Pe tor fereastra nu are margini, deci dacă e mai mare decât harta, harta se repetă.
class Viewport {
public:
	// Pornește cu toată harta în fereastră, o singură dată, cu originea în (0, 0).
	Viewport(const TorusGeometry& geometry, int cellWidth);  // aruncă ConfigError dacă cellWidth < 1

	[[nodiscard]] int columns() const;
	[[nodiscard]] int rows() const;
	[[nodiscard]] Coord center() const;

	// Celula de hartă afișată în fereastră la (column, row)
	[[nodiscard]] Coord worldAt(int column, int row) const;
	[[nodiscard]] Coord worldAt(Coord coord) const;

	void pan(Direction direction);                           // mută fereastra cu o celulă
	void panMore(Coord direction);                           // mută fereastra cu un vector
	void centerOn(const Coord& target);

	// Potrivește fereastra pe o zonă de terminal, păstrând centrul.
	void fitTo(int terminalColumns, int terminalRows);

	friend std::ostream& operator<<(std::ostream& os, const Viewport& viewport);

private:
	TorusGeometry geometry_;  // o copie pt că harta nu își schimbă dimensiunea
	int cellWidth_;
	Coord origin_, size_;
};
