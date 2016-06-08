#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>

#include <cinder/gl/gl.h>
#include <cinder/gl/Texture.h>

#include <cinder/Font.h>
#include <cinder/ImageIo.h>
#include <cinder/Unicode.h>

using namespace ci;
using namespace ci::app;
using namespace gl;
using namespace std;

class Sethex : public App {
	Font font;
	TextureRef texture;
	TextureFontRef texture_font;
public:
	void setup() override;
	void update() override;
	void draw() override;
	void mouseDown(MouseEvent event) override;
	void keyDown(KeyEvent event)override;
	void keyUp(KeyEvent event) override;
};

void Sethex::setup() {
	font = Font(loadAsset("fonts/Icomoon.ttf"), 20.0f);
	texture_font = TextureFont::create(font);
	texture = Texture::create(loadImage(loadAsset("images/Background.jpg")));
	console() << "Sethex by Thomas Würstle" << endl;
	enableVerticalSync(false);
}

void Sethex::update() {}

void Sethex::draw() {
	gl::clear(Color(0, 0, 0), true);
	gl::draw(texture);
	gl::drawString(u8"\ue000 Thomas Würstle", vec2(10, 10), ColorA(1, 1, 1), font);
	gl::drawString(to_string(static_cast<unsigned>(getAverageFps() + 0.5f)) + " FPS", vec2(5, getWindowHeight() - font.getAscent()));
}

void Sethex::mouseDown(MouseEvent event) {}

void Sethex::keyDown(KeyEvent event) {}

void Sethex::keyUp(KeyEvent event) {
	if (event.getCode() == KeyEvent::KEY_F11) {
		getWindow()->setFullScreen(!getWindow()->isFullScreen());
	}
}

CINDER_APP(Sethex, RendererGl, [&](App::Settings* settings) {
	settings->setWindowSize(1000, 600);
	settings->disableFrameRate();
})
