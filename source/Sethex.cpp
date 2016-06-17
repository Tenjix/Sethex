﻿#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <cinder/CameraUi.h>

#include <game/Game.h>

#include <utilities/cinder/OutputRedirection.h>

using namespace ci;
using namespace ci::app;
using namespace ci::utilities;
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

		void mouseMove(MouseEvent event) override {};
		void mouseDrag(MouseEvent event) override { camera_ui.mouseDrag(event); };
		void mouseDown(MouseEvent event) override { camera_ui.mouseDown(event); };
		void mouseUp(MouseEvent event) override { camera_ui.mouseUp(event); };
		void mouseWheel(MouseEvent event) override { camera_ui.mouseWheel(event); };
		void keyDown(KeyEvent event)override;
		void keyUp(KeyEvent event) override;

	};

	void Sethex::setup() {
		redirectOutputStreams(redirection_buffer);
		print("Sethex by Tenjix (Thomas Würstle)");
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
	}

}

CINDER_APP(sethex::Sethex, RendererGl, [&](App::Settings* settings) {
	settings->setWindowSize(1000, 560);
	settings->disableFrameRate();
})
