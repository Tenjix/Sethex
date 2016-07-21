#include <cinder/CameraUi.h>
#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <cinder/utilities/OutputRedirection.h>
#include <cinder/interface/CinderImGui.h>

#include <sethex/game/Game.h>

using namespace cinder;
using namespace cinder::app;
using namespace cinder::utilities;
using namespace std;

namespace sethex {

	class Sethex : public App {

		RedirectionBuffer redirection_buffer;
		CameraUi camera_ui;
		Game game;

	public:

		void setup() override;
		void resize() override;
		void update() override;
		void draw() override;

		void mouseMove(MouseEvent event) override { game.input.mouse.position = event.getPos(); };
		void mouseDrag(MouseEvent event) override { camera_ui.mouseDrag(event); };
		void mouseDown(MouseEvent event) override { camera_ui.mouseDown(event); };
		void mouseUp(MouseEvent event) override { camera_ui.mouseUp(event); };
		void mouseWheel(MouseEvent event) override { camera_ui.mouseWheel(event); };
		void keyDown(KeyEvent event)override;
		void keyUp(KeyEvent event) override;

	};

	void Sethex::setup() {
		redirectOutputStreams(redirection_buffer);
		ui::initialize(ui::Options().darkTheme().fonts({ { getAssetPath("fonts/Nunito.ttf"), 20.0f } }).fontGlyphRanges("Nunito", { 0x0020, 0x00FF, 0xe000, 0xe006, 0 }));
		getWindow()->setTitle("Sethex");
		print("Sethex by Tenjix (Thomas Würstle)");
		debug("Cinder Version ", CINDER_VERSION_STR);
		game.setup(camera_ui);
	}

	void Sethex::resize() {
		camera_ui.setWindowSize(getWindowSize());
		game.resize();
	}

	void Sethex::update() {
		game.update(static_cast<float>(getElapsedSeconds()), static_cast<unsigned>(getAverageFps() + 0.5f));
	}

	void Sethex::draw() {
		game.render();
	}

	void Sethex::keyDown(KeyEvent event) {}

	void Sethex::keyUp(KeyEvent event) {
		if (event.getCode() == KeyEvent::KEY_F11) {
			getWindow()->setFullScreen(!getWindow()->isFullScreen());
		}
		if (event.getCode() == KeyEvent::KEY_SPACE) {
			auto& camera = const_cast<CameraPersp&>(camera_ui.getCamera());
			camera.lookAt(vec3(0, 2.5, 2.5), vec3(0));
		}
	}

}

CINDER_APP(sethex::Sethex, RendererGl, [&](App::Settings* settings) {
	settings->setWindowSize(1000, 560);
	settings->disableFrameRate();
})
