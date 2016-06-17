#pragma once

#include <sethex/Common.h>
#include <sethex/EntitySystem.h>
#include <sethex/Graphics.h>

namespace sethex {

	class Material : public Component {

	public:

		SharedPointerProperty<Shader, Material> shader;
		SharedPointerProperty<Texture, Material> texture;

		Material() {
			shader._property_owner(this);
			texture._property_owner(this);
		}

	};

}
