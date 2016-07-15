#pragma once

#include <sethex/Common.h>
#include <sethex/EntitySystem.h>
#include <sethex/Graphics.h>

namespace sethex {

	class Geometry : public ObservableComponent {

	public:

		SharedProperty<Mesh, Geometry> mesh;

		Property<float3, Geometry> position;
		Property<quaternion, Geometry> rotation;
		Property<float3, Geometry> scaling;

		Geometry(const shared<Mesh>& mesh = nullptr) : mesh(mesh), scaling(float3(1.0f)) {
			this->mesh.owner = this;
			this->mesh.attach([this]() { notify(); });
			position.owner = this;
			rotation.owner = this;
			scaling.owner = this;
		}

		static shared<Geometry> create(const shared<Mesh>& mesh = nullptr) {
			return std::make_shared<Geometry>(mesh);
		}

		void render() {
			using namespace ci::gl;
			pushModelMatrix();
			translate(position);
			rotate(rotation);
			scale(scaling);
			draw(mesh);
			popModelMatrix();
		}

	};

}
