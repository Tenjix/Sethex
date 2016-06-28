#include "RenderSystem.h"

using namespace cinder::gl;

namespace sethex {

	void RenderSystem::render() {
		for (Entity entity : get_entities()) {
			Geometry& geometry = entity.get<Geometry>();
			Material& material = entity.get<Material>();
			material.bind();
			geometry.bind();
		}
	}

}
