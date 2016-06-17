#pragma once

#include <cinder/Font.h>
#include <cinder/ImageIo.h>
#include <cinder/Unicode.h>
#include <cinder/CameraUi.h>

#include <sethex/Common.h>
#include <sethex/EntitySystem.h>
#include <sethex/Graphics.h>

namespace sethex {

	class Game {

		World world;

		Font font;
		PerspectiveCamera camera;

		shared<FrameBuffer> framebuffer;
		shared<Texture> background;

		unsigned2 display_size;

		float time;
		float time_delta;
		unsigned frames_per_second;

	public:

		void setup(ci::CameraUi& camera_ui);
		void resize();
		void update(float elapsed_seconds, unsigned frames_per_second);
		void render();

	};

}
