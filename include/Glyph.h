#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>

// Cum arată ceva pe ecran (RGB + un simbol de o coloană, eventual completat)
class Glyph {
public:
	Glyph(std::string text, std::uint8_t red, std::uint8_t green, std::uint8_t blue);

	[[nodiscard]] const std::string& text() const;
	[[nodiscard]] std::uint8_t red() const;
	[[nodiscard]] std::uint8_t green() const;
	[[nodiscard]] std::uint8_t blue() const;

	// Același simbol și aceeași culoare, cu alt text
	[[nodiscard]] Glyph withText(const std::string& newText) const;

	friend std::ostream& operator<<(std::ostream& os, const Glyph& glyph);

private:
	std::string text_;
	std::uint8_t red_;
	std::uint8_t green_;
	std::uint8_t blue_;
};
