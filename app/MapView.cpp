#include "MapView.h"
#include "TorusGeometry.h"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

#include <ftxui/component/app.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/terminal.hpp>

namespace {
	// Închide aplicația ftxui după un număr de secunde, dacă nu a închis-o deja utilizatorul.
	class ExitTimer {
	public:
		ExitTimer(ftxui::App& app, int seconds) {
			if (seconds <= 0) return;
			thread_ = std::thread{[this, &app, seconds] {
				std::unique_lock lock{mutex_};
				if (!wake_.wait_for(lock, std::chrono::seconds{seconds}, [this] { return stopped_; })) app.Exit();
			}};
		}

		~ExitTimer() {
			{
				const std::lock_guard lock{mutex_};
				stopped_ = true;
			}
			wake_.notify_one();
			if (thread_.joinable()) thread_.join();
		}

		ExitTimer(const ExitTimer&) = delete;
		ExitTimer& operator=(const ExitTimer&) = delete;

	private:
		std::mutex mutex_;
		std::condition_variable wake_;
		bool stopped_ = false;
		std::thread thread_;  // ultimul membru: pornește după ce restul sunt construiți
	};

	ftxui::Color toColor(const Glyph& glyph) { return ftxui::Color::RGB(glyph.red(), glyph.green(), glyph.blue()); }
} // namespace

MapView::MapView(const WorldMap& map, const EntityKinds& kinds, const Legend& legend) :
	map_{map}, kinds_{kinds}, legend_{legend}, viewport_{map.geometry(), legend.cellWidth()} {}

const LayerProfile& MapView::profile() const { return legend_.profile(profile_); }
const Viewport& MapView::viewport() const { return viewport_; }

void MapView::nextProfile() { profile_ = (profile_ + 1) % legend_.profileCount(); }
void MapView::centerOn(const Coord& target) { viewport_.centerOn(target); }
void MapView::resize(int terminalColumns, int terminalRows) { viewport_.fitTo(terminalColumns, terminalRows); }
void MapView::resetViewport() { viewport_ = Viewport{map_.geometry(), legend_.cellWidth()}; }

ftxui::Element MapView::grid() const {
	const Coord cursor = viewport_.center();
	ftxui::Elements rows;
	for (int row = 0; row < viewport_.rows(); ++row) {
		ftxui::Elements cells;
		for (int column = 0; column < viewport_.columns(); ++column) {
			const Coord where = viewport_.worldAt(column, row);
			const Glyph glyph = legend_.look(map_.cellAt(where), kinds_, profile());

			auto cell = ftxui::text(glyph.text()) | ftxui::color(toColor(glyph));
			if (where == cursor) cell = cell | ftxui::inverted;
			cells.push_back(cell);
		}
		rows.push_back(ftxui::hbox(std::move(cells)));
	}
	return ftxui::vbox(std::move(rows));
}

ftxui::Element MapView::statusBar() const {
	const Coord cursor = viewport_.center();
	std::ostringstream position;
	position << "profil: " << profile().name() << " | cursor " << cursor << " | sageti: muta, Tab: profil, q: iesire";
	return ftxui::vbox({
		ftxui::text(position.str()) | ftxui::bold,
		ftxui::text(describe(map_.cellAt(cursor))) | ftxui::dim
	});
}

std::string MapView::describe(const Cell& cell) const {
	// pe scurt, cu tipurile din catalog, ca să încapă pe un rând: "animal 3/3: rabbit rabbit rabbit | ..."
	const std::string separators[2] = { "", " | " };
	std::ostringstream text;
	for (const Layer layer : {Layer::Animal, Layer::Object, Layer::Ground}) {
		const auto& ids = cell.occupants(layer);
		text << separators[(layer != Layer::Animal)] << layer << ' ' << ids.size() << '/' << cell.capacity(layer) << ':';
		if (ids.size() == 0) text << " -";
		for (const EntityId id : ids) text << ' ' << kinds_.kindOf(id).value_or("?");
	}
	return text.str();
}

ftxui::Element MapView::frame() const { return ftxui::vbox({grid(), statusBar()}); }

std::string MapView::snapshot() const {
	ftxui::Element document = frame();
	const int width = std::max(viewport_.columns() * legend_.cellWidth(), ftxui::Dimension::Fit(document).dimx);
	auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(width), ftxui::Dimension::Fixed(viewport_.rows() + statusRows));
	ftxui::Render(screen, document);
	return screen.ToString();
}

bool MapView::handle(const ftxui::Event& event) {
	if (event == ftxui::Event::ArrowUp) viewport_.pan(Direction::N);
	else if (event == ftxui::Event::ArrowDown) viewport_.pan(Direction::S);
	else if (event == ftxui::Event::ArrowLeft) viewport_.pan(Direction::W);
	else if (event == ftxui::Event::ArrowRight) viewport_.pan(Direction::E);
	else if (event == ftxui::Event::w) viewport_.panMore(TorusGeometry::offsetOf(Direction::N) * 4);
	else if (event == ftxui::Event::s) viewport_.panMore(TorusGeometry::offsetOf(Direction::S) * 4);
	else if (event == ftxui::Event::a) viewport_.panMore(TorusGeometry::offsetOf(Direction::W) * 4);
	else if (event == ftxui::Event::d) viewport_.panMore(TorusGeometry::offsetOf(Direction::E) * 4);
	else if (event == ftxui::Event::Tab) nextProfile();
	else return false;
	return true;
}

void MapView::interact(int seconds) {
	auto app = ftxui::App::Fullscreen();

	auto renderer = ftxui::Renderer([this] {
		const auto terminal = ftxui::Terminal::Size();
		resize(terminal.dimx, terminal.dimy - statusRows);
		return frame();
	});
	auto component = ftxui::CatchEvent(renderer, [this, &app](const ftxui::Event& event) {
		if (event == ftxui::Event::Character('q') || event == ftxui::Event::Escape) {
			app.Exit();
			return true;
		}
		return handle(event);
	});

	const ExitTimer timer{app, seconds};
	app.Loop(component);
}
