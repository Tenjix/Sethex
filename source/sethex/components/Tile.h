#pragma once

#include <hexagonal/Coordinates.h>

#include <sethex/EntitySystem.h>

namespace tenjix {

	namespace sethex {

		class Tile : public Component {

		public:

			hex::Coordinates coordinates;
			const char* biome = "";

		};

	}

}
