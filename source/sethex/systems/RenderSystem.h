#pragma once

#include <sethex/Common.h>
#include <sethex/Graphics.h>

#include <sethex/components/Display.h>
#include <sethex/components/Geometry.h>
#include <sethex/components/Instantiable.h>
#include <sethex/components/Material.h>

namespace sethex {

	class RenderSystem : public System {

		unordered_map<const Shader*, unordered_map<const Material*, unordered_map<const Mesh*, Entities>>> entity_mapping;
		unordered_map<Instantiable*, Entities> instantiables;

		void render(const Display& display);
		void render(const Entity& entity);

		bool validate(const Entity& entity, const Shader& mapped_shader, const Mesh& mapped_mesh);

		void map(const Shader& shader, const Material& material, const Mesh& mesh, const Entity& entity);
		void unmap(const Shader& shader, const Material& material, const Mesh& mesh, const Entity& entity);

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
