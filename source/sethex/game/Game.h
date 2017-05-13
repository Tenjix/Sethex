#pragma once

#define GLM_SWIZZLE

#include <cinder/Font.h>
#include <cinder/CameraUi.h>

#include <hexagonal/Map.h>

#include <sethex/Common.h>
#include <sethex/EntitySystem.h>
#include <sethex/Graphics.h>
#include <sethex/components/Display.h>
#include <sethex/world/Generator.h>

namespace tenjix {

	namespace sethex {

		class Game {

			World world;
			Generator generator;
			ci::CameraUi camera_ui;

			shared<TextureFont> font;
			Color font_color;

			shared<Texture> background;

			float time;
			float time_delta;
			unsigned frames_per_second;

			hexagonal::Map map;
			Lot<shared<Texture>> labels;

			String message;

		public:

			void setup(const shared<Window>& window);
			void resize();
			void update(float elapsed_seconds, unsigned frames_per_second);
			void render();

			void mouseMove(MouseEvent event);
			void mouseDrag(MouseEvent event);
			void mouseDown(MouseEvent event);
			void mouseUp(MouseEvent event);
			void mouseWheel(MouseEvent event);
			void keyDown(KeyEvent event);
			void keyUp(KeyEvent event);

		};

	}

}
