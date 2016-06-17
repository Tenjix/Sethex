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

		Property<shared<Mesh>, Geometry> mesh;

		Geometry() : scaling(float3(1.0f)) {
			position._property_owner(this);
			rotation._property_owner(this);
			scaling._property_owner(this);
			mesh._property_owner(this);
		}

	};

}
