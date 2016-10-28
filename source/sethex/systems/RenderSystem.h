#pragma once

#include <sethex/Common.h>
#include <sethex/Graphics.h>

#include <sethex/components/Display.h>
#include <sethex/components/Geometry.h>
#include <sethex/components/Instantiable.h>
#include <sethex/components/Material.h>

namespace sethex {

	class RenderSystem : public System {

		//using MeshMapping = unordered_map<linked<Mesh>, Entities, std::owner_less<linked<Mesh>>>;
		//using MaterialMapping = unordered_map<linked<Material>, MeshMapping, std::owner_less<linked<Material>>>;
		//using ShaderMapping = unordered_map<linked<Shader>, MaterialMapping, std::owner_less<linked<Shader>>>;
		//using InstantiableMapping = unordered_map<linked<Instantiable>, Entities, std::owner_less<linked<Instantiable>>>;
		using InstantiableMapping = unordered_map<Instantiable*, Entities>;

		//ShaderMapping entity_mapping;
		Entities uninstantiables;
		InstantiableMapping instantiables;

		void render(const Display& display);
		void render(const Entity& entity);
		void render(const Entity& entity, const shared<Shader>& mapped_shader, const shared<Material>& mapped_material, const shared<Mesh>& mapped_mesh);

		void map(const Entity& entity, const linked<Shader>& shader, const linked<Material>& material, const linked<Mesh>& mesh);
		void unmap(const Entity& entity, const linked<Shader>& shader, const linked<Material>& material, const linked<Mesh>& mesh);

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
