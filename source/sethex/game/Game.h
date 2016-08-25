#pragma once

#define GLM_SWIZZLE

#include <cinder/Font.h>
#include <cinder/CameraUi.h>

#include <sethex/Common.h>
#include <sethex/EntitySystem.h>
#include <sethex/Graphics.h>
#include <sethex/game/Input.h>
#include <sethex/components/Display.h>
#include <sethex/hexagonal/Map.h>
#include <sethex/world/Generator.h>

namespace sethex {

	class Game {

		World world;
		Generator generator;

		shared<TextureFont> font;
		Color font_color;

		shared<Texture> background;

		float time;
		float time_delta;
		unsigned frames_per_second;

		hexagonal::Map map;
		vector<shared<Texture>> labels;

	public:

		Input input;
		String message;

		void setup(ci::CameraUi& camera_ui);
		void resize();
		void update(float elapsed_seconds, unsigned frames_per_second);
		void render();

	};

}
