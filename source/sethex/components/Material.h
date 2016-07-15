#pragma once

#include <sethex/Common.h>
#include <sethex/EntitySystem.h>
#include <sethex/Graphics.h>

#include <utilities/Exceptions.h>

namespace sethex {

	class Material : public ObservableComponent {

	public:

		SharedProperty<Shader, Material> shader;
		Textures textures;

		Property<String, Material> name;
		Property<bool, Material> transparent;

		Material(const shared<Shader>& shader = nullptr) : shader(shader), transparent(false) {
			this->shader.owner = this;
			this->shader.attach([this]() { notify(); });
			name.owner = this;
			transparent.owner = this;
		}

		static shared<Material> create(const shared<Shader>& shader = nullptr) {
			return std::make_shared<Material>(shader);
		}

		Material& add(const shared<Texture>& texture) {
			textures.push_back(texture);
			return *this;
		}

		virtual void bind() {
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
