#pragma once

#include <cinder/gl/gl.h>
#include <cinder/gl/Texture.h>

#include <cinder/Font.h>
#include <cinder/ImageIo.h>
#include <cinder/Unicode.h>

#include <utilities/Standard.h>

class Game {

	ci::Font font;
	ci::CameraPersp camera;
	shared<ci::gl::GlslProg> shader;
	shared<ci::gl::Texture> texture;
	shared<ci::gl::TextureFont> texture_font;
	glm::ivec2 display_size;

	float time;
	float time_delta;
	unsigned frames_per_second;

public:

	void setup();
	void resize();
	void update(float elapsed_seconds, unsigned frames_per_second);
	void render();

};

