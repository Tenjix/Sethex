#include "Coordinates.h"

namespace tenjix {

	namespace hexagonal {

		const Coordinates Coordinates::Origin = Coordinates();
		const float2 Coordinates::Spacing = float2(UnitHexagon.width, UnitHexagon.height * 0.75f);

		const Coordinates Direction_Coordinates[6] {
			{ +1, -1 }, // NorthEast (dw=0)
			{ -1, 0 }, // East (dv=0)
			{ 0, -1 }, // SouthEast (du=0)
			{ -1, +1 }, // SouthWest (dw=0)
			{ -1, 0 }, // West (dv=0)
			{ 0, -1 } // NorthWest (du=0)
		};

		const Coordinates Heading_Coordinates[12] {
			{ +1, -2 }, // Northward (-v-axis)
			{ +2, -2 }, // NorthEastward (dw=0)
			{ +2, -1 }, // EastNorthward (+u-axis)
			{ +2, 0 }, // Eastward (dv=0)
			{ +1, +1 }, // EastSouthward (-w-axis)
			{ 0, +2 }, // SouthEastward (du=0)
			{ -1, +2 }, // Southward (+v-axis)
			{ -2, +2 }, // SouthWestward (dw=0)
			{ -2, +1 }, // WestSouthward (-u-axis)
			{ -2, 0 }, // Westward (dv=0)
			{ -1, -1 }, // WestNorthward (+w-axis)
			{ 0, -2 } // NorthWestward (du=0)
		};

		Lot<Coordinates> Coordinates::line(const Coordinates& begin, const Coordinates& end, bool supercover, bool edgecover) {
			Lot<Coordinates> line;
			if (begin == end) return line;

			// calculate length
			auto vector = end - begin;
			auto length = vector.magnitude();

			trace("line from ", begin, " to ", end, " (length=", length, ", supercover=", supercover, ", edgecover", edgecover, ")");

			auto general_direction = vector.general_direction();
			int rotation = hexagonal::rotation(general_direction, Direction::East);
			trace("general direction: ", general_direction, " (rotation=", rotation, ")");
			Direction straight = general_direction;
			Direction diagonal_up = hexagonal::rotate(straight, -1);
			Direction diagonal_down = hexagonal::rotate(straight, 1);
			Heading straight_up = hexagonal::rotate(vector.general_heading(), -2);
			vector.rotate(rotation);

			// calculate delta
			auto delta = vector.to_offset();

			// "2 * delta.x" and "delta.y" are always integers
			int dx = static_cast<int>(2 * delta.x);
			int dy = static_cast<int>(3 * delta.y);

			// "y = m * x"  with  "m = delta.y/delta.x * sqrt(3)/2"  and  "x = sqrt(3)/2"
			// "y = delta.y/delta.x * 3/4", so scale everything by "4*delta.x"="2*dx" to avoid division
			// "s = unit hexagon side length = 1.0" is threshold

			// run step-algorithm
			Coordinates current = begin;
			line.reserve(length + 1);
			line.push_back(begin);
			trace("begin ", begin);
			int y = 0;
			for (uint i = 1; i < length + 1; i++) {
				y += dy; // y += m * x
				int cover_y = y + dy; // y += m
				if (y >= dx) { // y > s/2 ?
					if (supercover) {
						if (edgecover ? (y == dx or cover_y <= 2 * dx) : (cover_y < 2 * dx)) { // y == s/2 or y + m <= s ?
							trace("cover straight");
							line.push_back(current + straight);
						} else if (edgecover ? (y >= 5 * dx) : (y > 5 * dx)) { // y >= 5 * s/2 ?
							trace("cover straight_up");
							line.push_back(current + straight_up);
						}
					}
					trace("go diagonal up");
					current += diagonal_up;
					y -= 3 * dx; // y -= 3 * s/2
				} else {
					if (supercover) {
						if (edgecover ? (abs(cover_y) >= 2 * dx) : (abs(cover_y) > 2 * dx)) { // |y + m| >= s ?
							trace("cover ", (cover_y > 0 ? "diagonal_up" : "diagonal_down"));
							line.push_back(current + (cover_y > 0 ? diagonal_up : diagonal_down));
						} else if (edgecover ? (y <= -dx) : (y < -dx)) { // y <= -s/2 ?
							trace("cover diagonal_down");
							line.push_back(current + diagonal_down);
						}
					}
					trace("go straight");
					current += straight;
					y += dy; // y += m
				}
				line.push_back(current);
				trace("step ", i, " ", current);
			}
			runtime_assert(current == end, "invalid end coordinates of hexagonal line algorithm");
			trace("end ", end);

			return line;
		}

		static void insert_ring(Lot<Coordinates>& Lot, unsigned radius, const Coordinates& center) {
			Coordinates coordinates = center + Direction::West * radius;
			for (unsigned direction = 0; direction < 6; direction++) {
				for (unsigned r = 0; r < radius; r++) {
					Lot.push_back(coordinates += static_cast<Direction>(direction) * 1);
				}
			}
		}

		Lot<Coordinates> Coordinates::ring(unsigned radius, const Coordinates& center) {
			if (radius == 0) return { center };
			Lot<Coordinates> ring;
			ring.reserve(6 * radius);
			insert_ring(ring, radius, center);
			return ring;
		}

		Lot<Coordinates> Coordinates::spiral(unsigned radius, const Coordinates& center) {
			if (radius == 0) return { center };
			Lot<Coordinates> spiral { center };
			spiral.reserve(1 + 3 * radius * (radius + 1)); // 1 + 6 * (1 + 2 + 3 + ...)
			for (unsigned ring_radius = 1; ring_radius <= radius; ring_radius++) {
				insert_ring(spiral, ring_radius, center);
			}
			return spiral;
		}

		Lot<Coordinates> Coordinates::rectangle(unsigned width, unsigned height, const Coordinates& origin, bool centered) {
			if (width == 0 and height == 0) return {};
			Lot<Coordinates> rectangle;
			rectangle.reserve(width * height);
			int v_begin = centered ? -static_cast<int>(height / 2) : 0;
			int v_end = centered ? v_begin + height : height;
			int u_begin = centered ? -static_cast<int>(width / 2) : 0;
			int u_end = centered ? u_begin + width : width;
			for (int v = v_begin; v < v_end; v++) {
				int offset = v / 2 - (v < 0 and is_odd(v));
				for (int u = u_begin - offset; u < u_end - offset; u++) {
					rectangle.push_back(origin + Coordinates(u, v));
				}
			}
			return rectangle;
		}

		String Coordinates::to_string(unsigned spacing) const {
			std::ostringstream stream;
			append(stream, *this, spacing);
			return stream.str();
		}

	};

}
