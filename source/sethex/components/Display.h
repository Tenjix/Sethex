#pragma once

#include <sethex/Common.h>
#include <sethex/EntitySystem.h>
#include <sethex/Graphics.h>

namespace tenjix {

	namespace sethex {

		class Display : public Component {

		public:

			shared<Window> window;
			PerspectiveCamera camera;
			unsigned2 size;
			shared<FrameBuffer> framebuffer;

			bool minimized() {
				return size.x == 0 or size.y == 0;
			}

		};

	}

}
