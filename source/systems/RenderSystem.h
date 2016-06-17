#pragma once

#include <components/Geometry.h>
#include <components/Material.h>

namespace sethex {

	class RenderSystem : public System {

	public:

		RenderSystem() : System(0) {
			filter.required_types.insert<Geometry, Material>();
		}

		void update(float delta_time) override {}; // do nothing on update
		void render();

	};

}
