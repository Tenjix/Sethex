#pragma once

#include <sethex/Common.h>
#include <sethex/EntitySystem.h>
#include <sethex/Graphics.h>

#include <utilities/Exceptions.h>

namespace sethex {

	class Material : public Component {

	public:

		Property<String, Material> name;
		SharedPointerProperty<Shader, Material> shader;
		Textures textures;

		Property<bool, Material> transparent = false;

		Material() {
			this->name._property_owner(this);
			this->shader._property_owner(this);
			this->transparent._property_owner(this);
		}

		static shared<Material> create() {
			return std::make_shared<Material>();
		}

		Material& add_texture(const shared<Texture>& texture) {
			textures.push_back(texture);
			return *this;
		}

		virtual void bind() {
			//if (gl::context()->getGlslProg() != shader().get())	
			shader->bind();
			bind_textures();
		}

		virtual void unbind() {
			unbind_textures();
		}

		virtual void bind_textures() {
			for (uint8 i = 0; i < number_of_textures(); i++) {
				textures[i]->bind(i);
			}
		}

		virtual void unbind_textures() {
			for (uint8 i = 0; i < number_of_textures(); i++) {
				textures[i]->unbind(i);
			}
		}

		uint8 number_of_textures() {
			uint n = textures.size();
			if (n > 32) throw_runtime_exception("too many textures");
			return static_cast<uint8>(n);
		}

	};

	class SmoothMaterial : public Material {

	};

	class RoughMaterial : public Material {

	};

}
