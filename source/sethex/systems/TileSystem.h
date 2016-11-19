#pragma once

#include <hexagonal/Map.h>

#include <sethex/Common.h>
#include <sethex/Graphics.h>
#include <sethex/components/Instantiable.h>
#include <sethex/components/Material.h>
#include <sethex/components/Tile.h>

namespace tenjix {

	namespace sethex {

		//using namespace tenjix;

		class TileSystem : public System {

			hex::Map map;
			Lot<Entity> tiles;

			bool focusing = false;

			float3 focus_position;
			float focus_expansion;
			Range<float> focus_range;

			shared<VertexBuffer> instance_positions;
			shared<VertexBuffer> instance_colors;

			ci::Shape2d hexagon_shape;
			float hexagon_extrusion = 5.0;

			shared<Instantiable> instantiable;
			shared<Material> material;
			shared<Mesh> mesh;

		public:

			hex::Coordinates focus_coordinates;
			hex::Coordinates previous_focus_coordinates;
			float3 target_focus_position;
			float3 previous_focus_position;
			optional<Tile> selected_tile;

			TileSystem() : System(1) {
				filter.required_types.insert<Tile>();
			}

			void mark(const Lot<hex::Coordinates>& coordinates);

			void initialize() override;
			void update(float delta_time) override;
			void update(Entity& entity, float delta_time) override;

			void resize(unsigned2 size);

			void update(shared<Surface> biome_map = nullptr, shared<Channel32f> elevation_map = nullptr, float scale = 1.0, float power = 1.0);
			void update(shared<ImageSource> biome_map = nullptr, shared<ImageSource> elevation_map = nullptr, float scale = 1.0, float power = 1.0) {
				if (not biome_map or not elevation_map) return;
				update(Surface::create(biome_map), Channel32f::create(elevation_map), scale, power);
			}

			optional<Entity> get_tile(float2 mouse_position) const;

			void focus(const hex::Coordinates& coordinates);
			void focus(const Entity& tile);
			void focus(const float3& position);

		};

	}

}
