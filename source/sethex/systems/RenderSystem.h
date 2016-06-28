#pragma once

#include <sethex/components/Geometry.h>
#include <sethex/components/Material.h>

namespace sethex {

	class RenderSystem : public System {

	public:

		RenderSystem() : System(0) {
			filter.required_types.insert<Geometry, Material>();
		}

		void update(float delta_time) override {};
		void render();

	};

}
