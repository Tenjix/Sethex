#include "RenderSystem.h"

#include <sethex/components/Tile.h>

using namespace cinder::gl;

namespace tenjix {

	namespace sethex {

		void RenderSystem::render() {
			trace("==================== render ====================");
			Display& display = world->find_entity("Main Display").get<Display>();
			render(display);
		}

		//template<class Key, class Value>
		//void iterate_map(std::unordered_map<Key, Value>& map, const std::function<bool(const Key& key, Value& value)>& erasure_check, const std::function<void(const Key& key, Value& value)>& operation) {
		//	auto iterator = map.begin();
		//	while (iterator != map.end()) {
		//		auto& entry = *iterator;
		//		if (erasure_check(entry.first, entry.second)) {
		//			operation(entry.first, entry.second);
		//			iterator++;
		//		} else {
		//			iterator = map.erase(iterator);
		//		}
		//	}
		//}

		void RenderSystem::render(const Display& display) {
			enableAlphaBlending();
			enableFaceCulling();
			// render into display framebuffer
			display.framebuffer->bindFramebuffer();
			setMatrices(display.camera);
			clear(Color(0, 0, 0, 0));
			enableDepth(true);

			//unordered_map<String, int> map;
			//iterate_map<String, int>(map, [](auto& key, auto& value) { return false; }, [](auto& key, auto& value) {});

			//auto& shader_mapping = entity_mapping;
			//auto shader_iterator = shader_mapping.begin();
			//while (shader_iterator != shader_mapping.end()) {
			//	auto shader = shader_iterator->first.lock();
			//	if (shader) {
			//		trace("bind ", shader->getLabel());
			//		shader->bind();
			//		auto& material_mapping = shader_iterator->second;
			//		auto material_iterator = material_mapping.begin();
			//		while (material_iterator != material_mapping.end()) {
			//			auto material = material_iterator->first.lock();
			//			if (material) {
			//				trace("bind textures of ", material->name);
			//				material->bind_textures();
			//				auto& mesh_mapping = material_iterator->second;
			//				auto mesh_iterator = mesh_mapping.begin();
			//				while (mesh_iterator != mesh_mapping.end()) {
			//					auto mesh = mesh_iterator->first.lock();
			//					if (mesh) {
			//						trace("render entities with mesh ", mesh.get());
			//						auto& entities = mesh_iterator->second;
			//						auto entity_iterator = entities.begin();
			//						while (entity_iterator != entities.end()) {
			//							auto& entity = *entity_iterator++;
			//							render(entity, shader, material, mesh);
			//						}
			//						mesh_iterator++;
			//					} else {
			//						mesh_iterator = mesh_mapping.erase(mesh_iterator);
			//					}
			//				}
			//				material->unbind_textures();
			//				material_iterator++;
			//			} else {
			//				material_iterator = material_mapping.erase(material_iterator);
			//			}
			//		}
			//		shader_iterator++;
			//	} else {
			//		shader_iterator = shader_mapping.erase(shader_iterator);
			//	}
			//}

			for (auto& entity : uninstantiables) {
				render(entity);
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
			material.bind();
			if (material.transparent) {
				cullFace(GL_FRONT);
				geometry.render();
				cullFace(GL_BACK);
				geometry.render();
			} else {
				geometry.render();
			}
			material.unbind();
		}

		void RenderSystem::render(const Entity& entity, const shared<Shader>& mapped_shader, const shared<Material>& mapped_material, const shared<Mesh>& mapped_mesh) {
			auto& material = entity.get<Material>();
			auto& geometry = entity.get<Geometry>();

			if (material.shader.pointer() != mapped_shader.get() or &material != mapped_material.get() or geometry.mesh.pointer() != mapped_mesh.get()) {
				unmap(entity, mapped_shader, mapped_material, mapped_mesh);
				return;
			}

			if (material.transparent) {
				cullFace(GL_FRONT);
				geometry.render();
				cullFace(GL_BACK);
				geometry.render();
			} else {
				geometry.render();
			}
		}

		void RenderSystem::map(const Entity& entity, const linked<Shader>& shader, const linked<Material>& material, const linked<Mesh>& mesh) {
			auto instantiable = entity.has<Instantiable>();
			if (instantiable) {
				instantiables[&instantiable.value()].insert(entity);
			} else {
				//entity_mapping[shader][material][mesh].insert(entity);
				uninstantiables.insert(entity);
			}
		}

		void RenderSystem::unmap(const Entity& entity, const linked<Shader>& shader, const linked<Material>& material, const linked<Mesh>& mesh) {
			auto instantiable = entity.has<Instantiable>();
			if (instantiable) {
				instantiables[&instantiable.value()].erase(entity);
			} else {
				//entity_mapping[shader][material][mesh].erase(entity);
				uninstantiables.erase(entity);
			}
		}

		void RenderSystem::on_entity_added(const Entity& entity) {
			auto& material = entity.get<Material>();
			auto& geometry = entity.get<Geometry>();
			material.attach(this, entity);
			geometry.attach(this, entity);
			map(entity, material.shader(), entity.get_reference<Material>(), geometry.mesh());
		}

		void RenderSystem::on_entity_removed(const Entity& entity) {
			auto& material = entity.get<Material>();
			auto& geometry = entity.get<Geometry>();
			material.detach(this, entity);
			geometry.detach(this, entity);
			unmap(entity, material.shader(), entity.get_reference<Material>(), geometry.mesh());
		}

		void RenderSystem::on_entity_modified(const Entity& entity) {
			// insert new mapping, old one will be removed by validate
			//auto& material = entity.get<Material>();
			//auto& geometry = entity.get<Geometry>();
			//invalid_entities.insert(entity);
			//map(entity, material.shader(), entity.get_reference<Material>(), geometry.mesh());
		}

	}

}
