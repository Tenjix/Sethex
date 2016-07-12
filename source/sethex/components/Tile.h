#pragma once

#include <sethex/EntitySystem.h>
#include <sethex/hexagonal/Map.h>

namespace sethex {

	class Tile : public Component {

	public:

		hexagonal::Coordinates coordinates;

	};

}
