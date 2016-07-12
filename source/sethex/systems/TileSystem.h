#pragma once

#include <sethex/Common.h>
#include <sethex/Graphics.h>
#include <sethex/game/Input.h>
#include <sethex/components/Tile.h>
#include <sethex/hexagonal/Map.h>

namespace sethex {

	class TileSystem : public System {

		const Input& input;

		hexagonal::Map map;

		bool active;
		float3 focus_position;
		float focus_expansion;
		Range<float> focus_range;
		hexagonal::Coordinates focus_coordinates;

	public:

		TileSystem(const Input& input) : System(1), input(input) {
			filter.required_types.insert<Tile>();
		}

		void TileSystem::initialize() override;
		void TileSystem::update(float delta_time) override;
		void TileSystem::update(Entity& entity, float delta_time) override;

	};

}
