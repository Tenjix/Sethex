#pragma once

#include <sethex/Common.h>
#include <sethex/Graphics.h>

#include <sethex/components/Display.h>
#include <sethex/components/Geometry.h>
#include <sethex/components/Material.h>

namespace sethex {

	class RenderSystem : public System {

		unordered_map<Shader*, unordered_map<Material*, unordered_map<Mesh*, Entities>>> entity_mapping;
		//unordered_map<pair<Shader*, Mesh*>, shared<Batch>> batch_mapping;

		void render(const Display& display);
		void render(const Entity& entity);

		bool validate(const Entity& entity, Shader* mapped_shader, Mesh* mapped_mesh);

		void on_entity_added(const Entity& entity) override;
		void on_entity_removed(const Entity& entity) override;
		void on_entity_modified(const Entity & entity) override;

	public:

		RenderSystem() : System(0) {
			filter.required_types.insert<Geometry, Material>();
		}

		void update(float delta_time) override {};
		void render();
	};

}
