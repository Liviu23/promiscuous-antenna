#include "Glyph.h"

#include <iomanip>
#include <ostream>
#include <utility>

Glyph::Glyph(std::string text, std::uint8_t red, std::uint8_t green, std::uint8_t blue) :
	text_{std::move(text)}, red_{red}, green_{green}, blue_{blue} {}

const std::string& Glyph::text() const { return text_; }
std::uint8_t Glyph::red() const { return red_; }
std::uint8_t Glyph::green() const { return green_; }
std::uint8_t Glyph::blue() const { return blue_; }

Glyph Glyph::withText(const std::string& newText) const { return Glyph{newText, red_, green_, blue_}; }

std::ostream& operator<<(std::ostream& os, const Glyph& glyph) {
	const auto flags = os.flags();
	os << '"' << glyph.text_ << "\" #" << std::hex << std::setfill('0');
	os << std::setw(2) << static_cast<int>(glyph.red_);
	os << std::setw(2) << static_cast<int>(glyph.green_);
	os << std::setw(2) <<  static_cast<int>(glyph.blue_);
	os.flags(flags);
	return os;
}
