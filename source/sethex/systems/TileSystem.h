#pragma once

#include <hexagonal/Map.h>

#include <sethex/Common.h>
#include <sethex/Graphics.h>
#include <sethex/components/Tile.h>

namespace tenjix {

	namespace sethex {

		//using namespace tenjix;

		class TileSystem : public System {

			hex::Map map;

			bool focusing = false;
			float3 target_focus_position;

			float3 focus_position;
			float focus_expansion;
			Range<float> focus_range;

			shared<VertexBuffer> instance_positions;
			shared<VertexBuffer> instance_colors;

		public:

			hex::Coordinates focus_coordinates;

			TileSystem() : System(1) {
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

}
