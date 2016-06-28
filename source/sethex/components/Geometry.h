#pragma once

#include <sethex/Common.h>
#include <sethex/EntitySystem.h>
#include <sethex/Graphics.h>

namespace sethex {

	class Geometry : public Component {

	public:

		Property<float3, Geometry> position;
		Property<quaternion, Geometry> rotation;
		Property<float3, Geometry> scaling;

		SharedPointerProperty<Mesh, Geometry> mesh;

		Geometry(const shared<Mesh>& mesh = nullptr) : scaling(float3(1.0f)), mesh(mesh) {
			position._property_owner(this);
			rotation._property_owner(this);
			scaling._property_owner(this);
			this->mesh._property_owner(this);
		}

		static shared<Geometry> create(const shared<Mesh>& mesh = nullptr) {
			return std::make_shared<Geometry>(mesh);
		}

		void bind() {
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
