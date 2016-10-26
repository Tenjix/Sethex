#include "RenderSystem.h"

#include <sethex/components/Tile.h>

using namespace cinder::gl;

namespace sethex {

	void RenderSystem::render() {
		trace("==================== render ====================");
		Display& display = world->find_entity("Main Display").get<Display>();
		render(display);
	}

	void RenderSystem::render(const Display& display) {
		enableAlphaBlending();
		enableFaceCulling();
		// render into display framebuffer
		display.framebuffer->bindFramebuffer();
		setMatrices(display.camera);
		clear(Color(0, 0, 0, 0));
		enableDepth(true);
		for (auto& shader_entry : entity_mapping) {
			auto& shader = *shader_entry.first;
			trace("bind ", shader.getLabel());
			shader.bind();
			for (auto& material_entry : shader_entry.second) {
				auto& material = *material_entry.first;
				trace("bind textures of ", material.name);
				material.bind_textures();
				for (auto& mesh_entiry : material_entry.second) {
					auto& mesh = *mesh_entiry.first;
					trace("render entities with mesh ", &mesh);
					auto& entities = mesh_entiry.second;
					auto iterator = entities.begin();
					while (iterator != entities.end()) {
						Entity entity = *iterator++;
						if (validate(entity, shader, mesh)) render(entity);
					}
				}
				material.unbind_textures();
			}
		}
		for (auto& instantiable_entry : instantiables) {
			auto& instantiable = *instantiable_entry.first;
			if (not instantiable.active) continue;
			auto& entities = instantiable_entry.second;
			auto entity = *entities.begin();
			auto& material = entity.get<Material>();
			material.bind_textures();
			instantiable.instantiate(entities.size());
			material.unbind_textures();
		}
		enableDepth(false);
		display.framebuffer->unbindFramebuffer();
		// draw display framebuffer
		setMatricesWindow(display.size);
		draw(display.framebuffer->getColorTexture());
	}

	void RenderSystem::render(const Entity& entity) {
		auto& material = entity.get<Material>();
		auto& geometry = entity.get<Geometry>();

		entity.has<Tile>().then([&material, &geometry](Tile& tile) {
			float3 color = glm::mix(float3(1.0), tile.coordinates.to_floats(), 0.25);
			material.shader()->uniform("uDiffuseColor", color);
		});

		if (material.transparent) {
			cullFace(GL_FRONT);
			geometry.render();
			cullFace(GL_BACK);
			geometry.render();
		} else {
			geometry.render();
		}
	}

	bool RenderSystem::validate(const Entity& entity, const Shader& mapped_shader, const Mesh& mapped_mesh) {
		auto& material = entity.get<Material>();
		auto& geometry = entity.get<Geometry>();
		runtime_assert(material.shader, entity, "'s material has no shader");
		if (material.shader != &mapped_shader or geometry.mesh != &mapped_mesh) {
			unmap(mapped_shader, material, mapped_mesh, entity);
			return false;
		}
		return true;
	}

	void RenderSystem::map(const Shader& shader, const Material& material, const Mesh& mesh, const Entity& entity) {
		auto instantiable = entity.has<Instantiable>();
		if (instantiable.exists()) {
			//auto& entry = instantiables.find(&instantiable.value());
			//if (entry == instantiables.end()) {
			//	instantiables.emplace(&instantiable.value(), std::initializer_list<Entity>{ entity });
			//} else {
			//	entry->second.insert(entity);
			//}
			instantiables[&instantiable.value()].insert(entity);
		} else {
			entity_mapping[material.shader.pointer()][&material][&mesh].insert(entity);
		}
	}

	void RenderSystem::unmap(const Shader& shader, const Material& material, const Mesh& mesh, const Entity& entity) {
		auto instantiable = entity.has<Instantiable>();
		if (instantiable.exists()) {
			instantiables[&instantiable.value()].erase(entity);
			//auto& entry = instantiables.find(&instantiable.value());
			//runtime_assert(entry != instantiables.end(), "");
			//entry->second.erase(entity);
			//if (entry->second.empty()) instantiables.erase(entry);
		} else {
			entity_mapping[material.shader.pointer()][&material][&mesh].erase(entity);
			//auto& shader_mapping = entity_mapping;
			//auto& shader_entry = shader_mapping.find(material.shader.pointer());
			//if (shader_entry != shader_mapping.end()) {
			//	auto& material_mapping = shader_entry->second;
			//	auto& material_entry = material_mapping.find(&material);
			//	if (material_entry != material_mapping.end()) {
			//		auto& mesh_mapping = material_entry->second;
			//		auto& mesh_entry = mesh_mapping.find(&mesh);
			//		if (mesh_entry != mesh_mapping.end()) {
			//			auto& entities = mesh_entry->second;
			//			entities.erase(entity);
			//			if (entities.empty()) mesh_mapping.erase(mesh_entry);
			//			if (mesh_mapping.empty()) material_mapping.erase(material_entry);
			//			if (material_mapping.empty()) shader_mapping.erase(shader_entry);
			//		}
			//	}
			//}
		}
	}

	void RenderSystem::on_entity_added(const Entity& entity) {
		auto& material = entity.get<Material>();
		auto& geometry = entity.get<Geometry>();
		material.attach(this, entity);
		geometry.attach(this, entity);
		map(material.shader, material, geometry.mesh, entity);
	}

	void RenderSystem::on_entity_removed(const Entity& entity) {
		auto& material = entity.get<Material>();
		auto& geometry = entity.get<Geometry>();
		material.detach(this, entity);
		geometry.detach(this, entity);
		unmap(material.shader, material, geometry.mesh, entity);
	}

	void RenderSystem::on_entity_modified(const Entity& entity) {
		// insert new mapping, old one will be removed by validate
		auto& material = entity.get<Material>();
		auto& geometry = entity.get<Geometry>();
		material.detach(this, entity);
		geometry.detach(this, entity);
		map(material.shader, material, geometry.mesh, entity);
	}

}
