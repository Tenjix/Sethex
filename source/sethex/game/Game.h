#pragma once

#include <cinder/Font.h>
#include <cinder/CameraUi.h>

#include <sethex/Common.h>
#include <sethex/EntitySystem.h>
#include <sethex/Graphics.h>

namespace sethex {

	class Game {

		World world;

		Font font;
		Color font_color;
		PerspectiveCamera camera;

		shared<FrameBuffer> framebuffer;
		shared<Texture> background;

		unsigned2 display_size;
		cinder::vec2 x;

		float time;
		float time_delta;
		unsigned frames_per_second;

	public:

		String message;

		void setup(ci::CameraUi& camera_ui);
		void resize();
		void update(float elapsed_seconds, unsigned frames_per_second);
		void render();

	};

}
