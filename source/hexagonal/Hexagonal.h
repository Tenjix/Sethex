#pragma once

#define GLM_SWIZZLE
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <utilities/Standard.h>
#include <utilities/Mathematics.h>
#include <utilities/Exceptions.h>

namespace tenjix {

	namespace hexagonal {

		using float2 = glm::vec2;
		using float3 = glm::vec3;

		struct Hexagon {

			// outer radius (distance from the center to the corners) = edge length of the hexagon's six equilateral triangles
			float outer_radius;
			// inner radius (minimal distance from the center to the edges) = height of the hexagon's six equilateral triangles
			float inner_radius;
			// size of the hexagon
			float2 size;
			// width of the hexagon
			float& width = size.x;
			// height of the hexagon
			float& height = size.y;
			// verices of the hexagon
			float2 vertices[6];

			explicit Hexagon(float side_length = 1.0f) {
				outer_radius = side_length;
				inner_radius = f::Sqrt_3 / 2 * side_length;
				size = { 2 * inner_radius, 2 * outer_radius };
				vertices[0] = { 0, +outer_radius };
				vertices[1] = { -inner_radius, +outer_radius / 2 };
				vertices[2] = { -inner_radius, -outer_radius / 2 };
				vertices[3] = { 0, -outer_radius };
				vertices[4] = { +inner_radius, -outer_radius / 2 };
				vertices[5] = { +inner_radius, +outer_radius / 2 };
			}

		};

		const Hexagon UnitHexagon;

		// Hexagonal axes
		enum class Axis : uint8 { U, V, W };

		// Hexagonal directions to all possible neighbor coordinates
		enum class Direction : uint8 {
			NorthEast, East, SouthEast, SouthWest, West, NorthWest
		};

		// Hexagonal headings to all possible secondary neighbor coordinates
		enum class Heading : uint8 {
			Northward, NorthEastward, EastNorthward, Eastward, EastSouthward, SouthEastward, Southward, SouthWestward, WestSouthward, Westward, WestNorthward, NorthWestward
		};

		// Rotates a direction clockwise by 60 degree per iteration.
		inline Direction rotate(Direction direction, int iterations = 1) {
			return static_cast<Direction>(project(static_cast<int>(direction) + iterations, 0, 5));
		}

		// Rotates a heading clockwise by 30 degree per iteration.
		inline Heading rotate(Heading heading, int iterations = 1) {
			return static_cast<Heading>(project(static_cast<int>(heading) + iterations, 0, 11));
		}

		inline Heading heading_of(Axis axis, bool positive = true) {
			switch (axis) {
				case Axis::U: return positive ? Heading::EastNorthward : Heading::WestSouthward;
				case Axis::V: return positive ? Heading::Southward : Heading::Northward;
				case Axis::W: return positive ? Heading::WestNorthward : Heading::EastSouthward;
				default: throw_runtime_exception();
			}
		}

		inline int rotation(Heading from, Heading to) {
			int delta = static_cast<int>(to) - static_cast<int>(from);
			return delta >= 0 ? delta : delta + 12;
		}

		inline int rotation(Direction from, Direction to) {
			int delta = static_cast<int>(to) - static_cast<int>(from);
			return delta >= 0 ? delta : delta + 6;
		}

		// elegant c-- way of converting enumerations to string

		inline std::ostream& operator<<(std::ostream& output, Direction direction) {
			switch (direction) {
				case Direction::NorthEast: return output << "NorthEast";
				case Direction::East: return output << "East";
				case Direction::SouthEast: return output << "SouthEast";
				case Direction::SouthWest: return output << "SouthWest";
				case Direction::West: return output << "West";
				case Direction::NorthWest: return output << "NorthWest";
			}
			return output << "No Direction";
		}

		inline std::ostream& operator<<(std::ostream& output, Heading heading) {
			switch (heading) {
				case Heading::Northward: return output << "Northward";
				case Heading::NorthEastward: return output << "NorthEastward";
				case Heading::EastNorthward: return output << "EastNorthward";
				case Heading::Eastward: return output << "Eastward";
				case Heading::EastSouthward: return output << "EastSouthward";
				case Heading::SouthEastward: return output << "SouthEastward";
				case Heading::Southward: return output << "Southward";
				case Heading::SouthWestward: return output << "SouthWestward";
				case Heading::WestSouthward: return output << "WestSouthward";
				case Heading::Westward: return output << "Westward";
				case Heading::WestNorthward: return output << "WestNorthward";
				case Heading::NorthWestward: return output << "NorthWestward";
			}
			return output << "No Heading";
		}

	}

	#ifndef HEXAGONAL_NO_NAMESPACE_ALIAS
	namespace hex = hexagonal;
	#endif

}