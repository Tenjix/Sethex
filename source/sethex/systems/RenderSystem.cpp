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
			Shader& shader = *shader_entry.first;
			trace("bind ", shader.getLabel());
			shader.bind();
			for (auto& material_entry : shader_entry.second) {
				auto& material = *material_entry.first;
				trace("bind textures of ", material.name);
				material.bind_textures();
				for (auto& mesh_entiry : material_entry.second) {
					auto& mesh = *mesh_entiry.first;
					trace("render entities with mesh ", &mesh);
					for (auto& entity : mesh_entiry.second) {
						render(entity);
					}
				}
				material.unbind_textures();
			}
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
			material.shader->uniform("uDiffuseColor", color);
		});

		if (material.transparent) {
			cullFace(GL_FRONT);
			geometry.render();
			cullFace(GL_BACK);
			geometry.render();
		} else {
			geometry.render();
		}

		//pair<Shader*, Mesh*> key { material.shader.address(), geometry.mesh.address() };
		//auto iterator = batch_mapping.find(key);
		//if (iterator == batch_mapping.end()) {
		//	iterator = batch_mapping.emplace(key, Batch::create(geometry.mesh, material.shader)).first;
		//}
		//auto& batch = *iterator->second;
		//batch.draw();
	}

	void RenderSystem::on_entity_added(const Entity& entity) {
		auto& material = entity.get<Material>();
		auto& geometry = entity.get<Geometry>();
		auto shader = material.shader.address();
		auto& textures = material.textures;
		auto mesh = geometry.mesh.address();
		entity_mapping[shader][&material][mesh].insert(entity);
	}

	void RenderSystem::on_entity_removed(const Entity& entity) {
		auto& material = entity.get<Material>();
		auto& geometry = entity.get<Geometry>();
		auto shader = material.shader.address();
		auto& textures = material.textures;
		auto mesh = geometry.mesh.address();
		entity_mapping[shader][&material][mesh].erase(entity);
	}

}
