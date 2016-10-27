#pragma once

#include <sethex/Common.h>
#include <sethex/Graphics.h>
#include <sethex/game/Input.h>
#include <sethex/components/Tile.h>
#include <sethex/hexagonal/Map.h>

namespace sethex {

	class TileSystem : public System {

		const Input& input;

		hex::Map map;

		bool active;
		float3 focus_position;
		float focus_expansion;
		Range<float> focus_range;
		hex::Coordinates focus_coordinates;

		shared<VertexBuffer> instance_positions;
		shared<VertexBuffer> instance_colors;

	public:

		TileSystem(const Input& input) : System(1), input(input) {
			filter.required_types.insert<Tile>();
		}

		void initialize() override;
		void update(float delta_time) override;
		void update(Entity& entity, float delta_time) override;

		void update(shared<Surface32f> biome_map = nullptr, shared<Channel32f> elevation_map = nullptr);
		void update(shared<ImageSource> biome_map = nullptr, shared<ImageSource> elevation_map = nullptr) {
			update(Surface32f::create(biome_map), Channel32f::create(elevation_map));
		}

	};

}
