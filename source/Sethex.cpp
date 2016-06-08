#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>

#include <game/Game.h>

using namespace ci;
using namespace ci::app;
using namespace gl;
using namespace std;

class Sethex : public App {

	Game game;

public:

	void setup() override;
	void resize() override;
	void update() override;
	void draw() override;

	void mouseDown(MouseEvent event) override;
	void keyDown(KeyEvent event)override;
	void keyUp(KeyEvent event) override;

};

void Sethex::setup() {
	console() << "Sethex by Tenjix (Thomas Würstle)" << endl;
	game.setup();
}

void Sethex::resize() {
	game.resize();
}

void Sethex::update() {
	game.update(static_cast<float>(getElapsedSeconds()), static_cast<unsigned>(getAverageFps() + 0.5f));
}

void Sethex::draw() {
	game.render();
}

void Sethex::mouseDown(MouseEvent event) {}

void Sethex::keyDown(KeyEvent event) {}

void Sethex::keyUp(KeyEvent event) {
	if (event.getCode() == KeyEvent::KEY_F11) {
		getWindow()->setFullScreen(!getWindow()->isFullScreen());
	}
}

CINDER_APP(Sethex, RendererGl, [&](App::Settings* settings) {
	settings->setWindowSize(1000, 560);
	settings->disableFrameRate();
})
