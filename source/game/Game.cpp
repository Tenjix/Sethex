#include "Game.h"

#include <cinder/app/App.h>

using namespace ci;
using namespace ci::app;
using namespace gl;
using namespace std;

void Game::setup() {
	font = Font(loadAsset("fonts/Icomoon.ttf"), 20.0f);
	texture = Texture::create(loadImage(loadAsset("images/Background.jpg")));
	shader = getStockShader(ShaderDef().lambert());
	camera.lookAt(vec3(5), vec3(0));
	enableVerticalSync(false);
}

void Game::resize() {
	camera.setAspectRatio(getWindowAspectRatio());
	display_size = getWindowSize();
}

void Game::update(float elapsed_seconds, unsigned frames_per_second) {
	this->frames_per_second = frames_per_second;
	time_delta = elapsed_seconds;
	time += time_delta;
}


void Game::render() {
	clear();

	setMatricesWindow(display_size);
	draw(texture);

	enableDepth(true);
	setMatrices(camera);
	shader->bind();
	drawSphere(vec3(), 1.0f, 32);
	enableDepth(false);

	setMatricesWindow(display_size);
	drawString(u8"\ue000 Sethex", vec2(10, 10), ColorA(1, 1, 1), font);
	drawString(u8"\ue001 \ue002 \ue003 \ue004 \ue005 \ue006", vec2(10, 40), ColorA(1, 1, 1), font);
	drawString(to_string(frames_per_second) + " FPS", vec2(5, display_size.y - font.getAscent()));
	drawStringRight(u8"Thomas Würstle", vec2(display_size.x - 5, display_size.y - font.getAscent()));
}
