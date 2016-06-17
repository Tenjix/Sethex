#include "RenderSystem.h"

using namespace cinder::gl;

namespace sethex {

	void RenderSystem::render() {
		for (Entity entity : get_entities()) {
			Geometry& geometry = entity.get<Geometry>();
			Material& material = entity.get<Material>();

			material.texture->bind();
			material.shader->bind();

			pushModelMatrix();

			translate(geometry.position);
			rotate(geometry.rotation);
			scale(geometry.scaling);
			draw(geometry.mesh);

			popModelMatrix();
		}
	}

}
