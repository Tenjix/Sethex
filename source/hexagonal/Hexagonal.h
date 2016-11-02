#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <utilities/Standard.h>
#include <utilities/Mathematics.h>

namespace tenjix {

	namespace hexagonal {

		using float2 = glm::vec2;
		using float3 = glm::vec3;

		enum class Direction : uint8 {
			NorthEast, East, SouthEast, SouthWest, West, NorthWest
		};

		enum class Heading : uint8 {
			Northward, NorthEastward, EastNorthward, Eastward, EastSouthward, SouthEastward, Southward, SouthWestward, WestSouthward, Westward, WestNorthward, NorthWestward
		};

	}

	#ifndef HEXAGONAL_NO_NAMESPACE_ALIAS
	namespace hex = hexagonal;
	#endif

}