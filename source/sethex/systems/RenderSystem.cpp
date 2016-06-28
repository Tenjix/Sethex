#include "RenderSystem.h"

using namespace cinder::gl;

namespace sethex {

	void RenderSystem::render() {
		enableAlphaBlending();
		enableFaceCulling();
		for (Entity entity : get_entities()) {
			Geometry& geometry = entity.get<Geometry>();
			Material& material = entity.get<Material>();
			material.bind();
			if (not material.shader) getStockShader(ShaderDef())->bind();
			if (material.transparent) {
				cullFace(GL_FRONT);
				geometry.render();
				cullFace(GL_BACK);
				geometry.render();
			} else {
				geometry.render();
			}

		}
	}

}
