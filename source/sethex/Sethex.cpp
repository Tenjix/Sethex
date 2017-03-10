#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <cinder/interface/Imgui.h>
#include <cinder/utilities/OutputRedirection.h>

#include <sethex/game/Game.h>

using namespace cinder;
using namespace cinder::app;
using namespace cinder::utilities;
using namespace std;

namespace tenjix {

	namespace sethex {

		class Sethex : public App {

			RedirectionBuffer redirection_buffer;
			CameraUi camera_ui;
			Timer timer;
			Game game;

			bool executable;
			String error;

			struct Version {
				pair<int, int> version;
				Version(const pair<int, int>& version) : version(version) {}
				bool exceeds(const pair<int, int>& other) {
					return other.first < version.first or other.first == version.first and other.second <= version.second;
				}
			};

		public:

			void setup() override;
			void resize() override;
			void update() override;
			void draw() override;

		};

		void Sethex::setup() {
			//redirectOutputStreams(redirection_buffer);
			getWindow()->setTitle("Sethex");
			print("Sethex by Tenjix (Thomas Würstle)");
			print("Cinder Version ", CINDER_VERSION_STR);
			print("OpenGL Version ", cinder::gl::getVersionString());
			bool opengl_version_supported = Version(cinder::gl::getVersion()).exceeds({ 3, 3 });
			if (not opengl_version_supported) error = "Error: Unsupported opengl version. \nDetected: OpenGL " + cinder::gl::getVersionString() + "\nRequired: OpenGL 3.3";
			ui::initialize(ui::Options().darkTheme().fonts({ { getAssetPath("fonts/Nunito.ttf"), 20.0f } }).fontGlyphRanges("Nunito", { 0x0020, 0x00FF, 0xe000, 0xe006, 0 }));
			executable = opengl_version_supported;
			if (executable) game.setup(getWindow());
			timer.start();
		}

		void Sethex::resize() {
			if (executable) game.resize();
		}

		void Sethex::update() {
			if (executable) game.update(static_cast<float>(timer.getSeconds()), static_cast<unsigned>(roundf(getAverageFps())));
			timer.start();
		}

		void Sethex::draw() {
			if (executable) {
				//{
				//	ui::SetNextWindowPosCenter();
				//	ui::ScopedWindow menu("Main Menu", ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
				//	ImVec2 size = { 200, 0 };
				//	ui::Button("Generate World", size);
				//	ui::Button("Play the Game", size);
				//	ui::Button("Customize Settings", size);
				//}
				game.render();
			} else {
				ui::SetNextWindowPosCenter();
				ui::ScopedWindow menu("Error", ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
				ui::Text(error);
			}
		}

	}

}

CINDER_APP(tenjix::sethex::Sethex, RendererGl(RendererGl::Options().version(4, 5)), [&](App::Settings* settings) {
	settings->setWindowSize(1000, 560);
	settings->setConsoleWindowEnabled();
	settings->disableFrameRate();
})
