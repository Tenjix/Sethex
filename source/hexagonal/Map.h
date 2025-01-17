#pragma once

#include <hexagonal/Coordinates.h>

#include <utilities/Optional.h>
#include <utilities/Mathematics.h>

namespace tenjix {

	namespace hexagonal {

		class Map {

			int u_begin;
			int u_end;
			int v_begin;
			int v_end;

			int u_offset(int v) const {
				return v / 2 - (v < 0 and is_odd(v));
			}

			bool contains_horizontally(int u, int offset) const {
				return u >= u_begin - offset and u <= u_end - offset;
			}

			bool contains_vertically(int v) const {
				return v >= v_begin and v <= v_end;
			}

		public:

			ReadonlyProperty<unsigned, Map> width;
			ReadonlyProperty<unsigned, Map> height;
			ReadonlyProperty<Lot<Coordinates>, Map> coordinates;
			ReadonlyProperty<unsigned2, Map> size;
			ReadonlyProperty<float2, Map> cartesian_size;

			// calculates the array index for given coordinates
			unsigned index(const Coordinates& coordinates) const {
				Coordinates reprojected = reproject(coordinates);
				unsigned u = reprojected.u - (u_begin - u_offset(reprojected.v));
				unsigned v = reprojected.v - v_begin;
				return u + v * width;
			}

			Map(unsigned width = 0, unsigned height = 0) : width(width), height(height), size({ width, height }), cartesian_size(float2(width, height) * Coordinates::Spacing) {
				this->width.owner = this;
				this->height.owner = this;
				coordinates.owner = this;
				size.owner = this;
				cartesian_size.owner = this;

				u_begin = -static_cast<int>(width / 2);
				u_end = u_begin + width - 1;
				v_begin = -static_cast<int>(height / 2);
				v_end = v_begin + height - 1;

				coordinates->reserve(width * height);
				for (int v = v_begin; v <= v_end; v++) {
					int offset = u_offset(v);
					for (int u = u_begin - offset; u <= u_end - offset; u++) {
						coordinates->push_back(Coordinates(u, v));
					}
				}
			}

			// determines whether "coordinates" lie within the horizontal limits of the map
			bool contains_horizontally(Coordinates coordinates) const {
				return contains_horizontally(coordinates.u, u_offset(coordinates.v));
			}

			// determines whether "coordinates" lie within the vertical limits of the map
			bool contains_vertically(Coordinates coordinates) const {
				return contains_vertically(coordinates.v);
			}

			// determines whether "coordinates" lie within the horizontal and vertical limits of the map
			bool contains(Coordinates coordinates) const {
				return contains_vertically(coordinates) and contains_horizontally(coordinates);
			}

			optional<Coordinates> neighbor(const Coordinates& coordinates, Direction direction) const {
				Coordinates neighbor_coordinates = coordinates.neighbor(direction);
				if (not contains_vertically(neighbor_coordinates)) return {};
				return reproject(neighbor_coordinates);
			}

			// reprojects "coordinates" into the map by wrapping horizontally and vertically
			Coordinates reproject(const Coordinates& coordinates) const {
				int u = coordinates.u;
				int v = coordinates.v;
				int offset = u_offset(v);
				if (not contains_horizontally(u, offset)) {
					u = project(u, u_begin - offset, u_end - offset);
				}
				if (not contains_vertically(v)) {
					v = project(v, v_begin, v_end);
					u -= u_offset(v) - offset;
				}
				return Coordinates(u, v);
			}

			float2 texinates(const Coordinates& coordinates) const {
				return 0.5f + coordinates.to_cartesian(Handedness::Left) / cartesian_size();
			}

		};

	}

}
