#pragma once

#include <sethex/Common.h>
#include <sethex/EntitySystem.h>
#include <sethex/Graphics.h>

namespace sethex {

	class Display : public Component {

	public:

		PerspectiveCamera camera;
		unsigned2 size;
		shared<FrameBuffer> framebuffer;

	};

}
