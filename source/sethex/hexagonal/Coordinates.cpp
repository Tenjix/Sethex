#include "Coordinates.h"

namespace sethex {

	namespace hexagonal {

		const Coordinates Coordinates::Origin = Coordinates();

		static void insert_ring(vector<Coordinates>& vector, unsigned radius, const Coordinates& center) {
			Coordinates coordinates = center + Direction::West * radius;
			for (unsigned direction = 0; direction < 6; direction++) {
				for (unsigned r = 0; r < radius; r++) {
					vector.push_back(coordinates += static_cast<Direction>(direction) * 1);
				}
			}
		}

		vector<Coordinates> Coordinates::ring(unsigned radius, const Coordinates& center) {
			if (radius == 0) return{ center };
			vector<Coordinates> ring;
			ring.reserve(6 * radius);
			insert_ring(ring, radius, center);
			return ring;
		}

		vector<Coordinates> Coordinates::spiral(unsigned radius, const Coordinates& center) {
			if (radius == 0) return{ center };
			vector<Coordinates> spiral{ center };
			spiral.reserve(1 + 3 * radius * (radius + 1)); // 1 + 6 * (1 + 2 + 3 + ...)
			for (unsigned ring_radius = 1; ring_radius <= radius; ring_radius++) {
				insert_ring(spiral, ring_radius, center);
			}
			return spiral;
		}

		vector<Coordinates> Coordinates::rectangle(unsigned width, unsigned height, const Coordinates& origin, bool centered) {
			vector<Coordinates> rectangle;
			rectangle.reserve(width * height);
			if (width == 0 and height == 0) return rectangle;
			int v_begin = centered ? -static_cast<int>(height / 2) : 0;
			int v_end = centered ? v_begin + height : height;
			int u_begin = centered ? -static_cast<int>(width / 2) : 0;
			int u_end = centered ? u_begin + width : width;
			for (int v = v_begin; v < v_end; v++) {
				int shift = v / 2 - (v % 2 == -1);
				for (int u = u_begin - shift; u < u_end - shift; u++) {
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
