#pragma once

#include <sethex/hexagonal/Coordinates.h>

#include <utilities/Optional.h>
#include <utilities/Mathematics.h>

namespace sethex {

	namespace hexagonal {

		class Map {

			int u_begin;
			int u_end;
			int v_begin;
			int v_end;

			vector<Coordinates> content;

			int u_offset(int v) const {
				return v / 2 - (v < 0 and odd(v));
			}

			bool within_horizontal_limits(int u, int offset) const {
				return u >= u_begin - offset and u <= u_end - offset;
			}

			bool within_vertical_limits(int v) const {
				return v >= v_begin and v <= v_end;
			}

		public:

			ReadonlyProperty<unsigned, Map> width;
			ReadonlyProperty<unsigned, Map> height;
			//ReadonlyProperty<vector<Coordinates>, Map> coordinates;

			const vector<Coordinates>& coordinates() const {
				return content;
			}

			// calculates the array index for given coordinates
			unsigned index(const Coordinates& coordinates) const {
				Coordinates reprojected = reproject(coordinates);
				unsigned u = reprojected.u - (u_begin - u_offset(reprojected.v));
				unsigned v = reprojected.v - v_begin;
				return u + v * width;
			}

			Map() : width(), height(), u_begin(), u_end(), v_begin(), v_end() {
				width.object = this;
				height.object = this;
			}

			Map(unsigned width, unsigned height) : width(width), height(height) {
				this->width.object = this;
				this->height.object = this;
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

			bool within_horizontal_limits(Coordinates coordinates) const {
				return within_horizontal_limits(coordinates.u, u_offset(coordinates.v));
			}

			bool within_vertical_limits(Coordinates coordinates) const {
				return within_vertical_limits(coordinates.v);
			}

			bool contains(Coordinates coordinates) const {
				return within_vertical_limits(coordinates) and within_horizontal_limits(coordinates);
			}

			Potential<Coordinates> neighbor(const Coordinates& coordinates, Direction direction) const {
				Coordinates neighbor_coordinates = coordinates.neighbor(direction);
				if (not within_vertical_limits(neighbor_coordinates)) return nullptr;
				return reproject(neighbor_coordinates);
			}

			// reprojects "coordinates" into the map by wrapping horizontally and vertically
			Coordinates reproject(const Coordinates& coordinates) const {
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