#pragma once

#include <sethex/hexagonal/Coordinates.h>

#include <utilities/Optional.h>
#include <utilities/Mathematics.h>

namespace sethex {

	namespace hexagonal {

		class Map {

			unsigned width;
			unsigned height;

			int u_begin;
			int u_end;
			int v_begin;
			int v_end;

			vector<Coordinates> content;

			int u_offset(int v) {
				return v / 2 - (v < 0 and odd(v));
			}

			bool within_horizontal_limits(int u, int offset) {
				return u >= u_begin - offset and u <= u_end - offset;
			}

			bool within_vertical_limits(int v) {
				return v >= v_begin and v <= v_end;
			}

		public:

			const vector<Coordinates>& coordinates() const {
				return content;
			}

			Map() : width(), height(), u_begin(), u_end(), v_begin(), v_end() {}

			Map(unsigned width, unsigned height) : width(width), height(height) {
				u_begin = -static_cast<int>(width / 2);
				u_end = u_begin + width - 1;
				v_begin = -static_cast<int>(height / 2);
				v_end = v_begin + height - 1;
				content.reserve(width * height);
				for (int v = v_begin; v <= v_end; v++) {
					int offset = u_offset(v);
					for (int u = u_begin - offset; u <= u_end - offset; u++) {
						content.push_back(Coordinates(u, v));
					}
				}
			}

			bool within_horizontal_limits(Coordinates coordinates) {
				return within_horizontal_limits(coordinates.u, u_offset(coordinates.v));
			}

			bool within_vertical_limits(Coordinates coordinates) {
				return within_vertical_limits(coordinates.v);
			}

			bool contains(Coordinates coordinates) {
				return within_vertical_limits(coordinates) and within_horizontal_limits(coordinates);
			}

			Potential<Coordinates> neighbor(const Coordinates& coordinates, Direction direction) {
				Coordinates neighbor_coordinates = coordinates.neighbor(direction);
				if (not within_vertical_limits(neighbor_coordinates)) return nullptr;
				return reproject(neighbor_coordinates);
			}

			// reprojects "coordinates" into the map by wrapping horizontally and vertically
			Coordinates reproject(const Coordinates& coordinates) {
				int u = coordinates.u;
				int v = coordinates.v;
				int offset = u_offset(v);
				if (not within_horizontal_limits(u, offset)) {
					u = project(u, u_begin - offset, u_end - offset);
				}
				if (not within_vertical_limits(v)) {
					v = project(v, v_begin, v_end);
					u -= u_offset(v) - offset;
				}
				return Coordinates(u, v);
			}

		};

	}

}