#include <cinder/CameraUi.h>
#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <cinder/utilities/OutputRedirection.h>
#include <cinder/interface/Imgui.h>

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

		String error;

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
		getWindow()->setTitle("Sethex");
		print("Sethex by Tenjix (Thomas Würstle)");
		debug("Cinder Version ", CINDER_VERSION_STR);
		auto opengl_version = cinder::gl::getVersion();
		bool opengl_version_supported = opengl_version.first > 3 or (opengl_version.first == 3 and opengl_version.second >= 2);
		debug("OpenGL Version ", cinder::gl::getVersionString(), opengl_version_supported ? " (supported)" : " (unsupported)");
		if (not opengl_version_supported) error = "Error: Unsupported opengl version. \nDetected: OpenGL " + cinder::gl::getVersionString() + "\nRequired: OpenGL 3.2 or higher";
		ui::initialize(ui::Options().darkTheme().fonts({ { getAssetPath("fonts/Nunito.ttf"), 20.0f } }).fontGlyphRanges("Nunito", { 0x0020, 0x00FF, 0xe000, 0xe006, 0 }));
		game.executable = opengl_version_supported;
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
		//{
		//	ui::SetNextWindowPosCenter();
		//	ui::ScopedWindow menu("Main Menu", ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
		//	ImVec2 size = { 200, 0 };
		//	ui::Button("Generate World", size);
		//	ui::Button("Play the Game", size);
		//	ui::Button("Customize Settings", size);
		//}
		game.render();
		if (not game.executable and not error.empty()) {
			ui::SetNextWindowPosCenter();
			ui::ScopedWindow menu("Error", ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
			ui::Text(error);
		}
	}

	void Sethex::keyDown(KeyEvent event) {}

	void Sethex::keyUp(KeyEvent event) {
		if (event.getCode() == KeyEvent::KEY_F11) {
			getWindow()->setFullScreen(!getWindow()->isFullScreen());
		}
		if (event.getCode() == KeyEvent::KEY_HOME) {
			auto& camera = const_cast<CameraPersp&>(camera_ui.getCamera());
			camera.lookAt(vec3(0, 2.5, 2.5), vec3(0));
		}
		if (event.getCode() == KeyEvent::KEY_SPACE) {
			auto& camera = const_cast<CameraPersp&>(camera_ui.getCamera());
			camera.lookAt(vec3(0, 250, 0.001), vec3(0));
		}
	}

}

CINDER_APP(sethex::Sethex, RendererGl(RendererGl::Options().version(4, 5)), [&](App::Settings* settings) {
	settings->setWindowSize(1000, 560);
	settings->disableFrameRate();
})
