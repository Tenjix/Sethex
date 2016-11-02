#pragma once

#include <sethex/Common.h>
#include <sethex/Graphics.h>

namespace tenjix {

	namespace sethex {

		class Generator {

		public:

			shared<ImageSource> biomes;
			shared<ImageSource> elevation;

			void display();

		};

	}

}
