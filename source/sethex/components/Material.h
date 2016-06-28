#pragma once

#include <sethex/Common.h>
#include <sethex/EntitySystem.h>
#include <sethex/Graphics.h>

#include <utilities/Exceptions.h>

namespace sethex {

	class Material : public Component {

	public:

		SharedPointerProperty<Shader, Material> shader;
		std::vector<shared<Texture>> textures;

		Material(const shared<Shader>& shader = nullptr) : shader(shader) {
			this->shader._property_owner(this);
		}

		static shared<Material> create(const shared<Shader>& shader = nullptr) {
			return std::make_shared<Material>(shader);
		}

		Material& add_texture(const shared<Texture>& texture) {
			textures.push_back(texture);
			return *this;
		}

		virtual void bind() {
			shader->bind();
			for (uint8 i = 0; i < number_of_textures(); i++) {
				textures[i]->bind(i);
			}
		}

		virtual void unbind() {
			for (uint8 i = 0; i < number_of_textures(); i++) {
				textures[i]->unbind(i);
			}
		}

		uint8 number_of_textures() {
			uint n = textures.size();
			if (n > 32) throw_("too many textures");
			return static_cast<uint8>(n);
		}

	};

	class AdvancedMaterial : public Material {

		bool initialized = false;

	public:

		SharedPointerProperty<Texture, Material> diffuse_texture;
		SharedPointerProperty<Texture, Material> specular_texture;
		SharedPointerProperty<Texture, Material> emissive_texture;
		SharedPointerProperty<Texture, Material> normal_texture;

		AdvancedMaterial(const shared<Shader>& shader = nullptr) : Material(shader) {
			this->diffuse_texture._property_owner(this);
			this->specular_texture._property_owner(this);
			this->emissive_texture._property_owner(this);
			this->normal_texture._property_owner(this);
		}

		void initialize() {
			if (initialized) return;
			initialized = true;
			shader->uniform("uDiffuseTexture", 0);
			shader->uniform("uSpecularTexture", 1);
			shader->uniform("uEmissiveTexture", 2);
			shader->uniform("uNormalMap", 3);
		}

		virtual void bind() {
			initialize();
			shader->bind();
			if (diffuse_texture) diffuse_texture->bind(0);
			if (specular_texture) specular_texture->bind(1);
			if (emissive_texture) emissive_texture->bind(2);
			if (normal_texture) normal_texture->bind(3);
		}

		virtual void unbind() {
			diffuse_texture->unbind(0);
			specular_texture->unbind(1);
			emissive_texture->unbind(2);
			normal_texture->unbind(3);
		}

	};

	class SmoothMaterial : public Material {

	};

	class RoughMaterial : public Material {

	};

}
