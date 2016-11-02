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

			Lot<Coordinates> content;

			int u_offset(int v) const {
				return v / 2 - (v < 0 and odd(v));
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
			//ReadonlyProperty<Lot<Coordinates>, Map> coordinates;

			const Lot<Coordinates>& coordinates() const {
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
				width.owner = this;
				height.owner = this;
			}

			Map(unsigned width, unsigned height) : width(width), height(height) {
				this->width.owner = this;
				this->height.owner = this;
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

			Potential<Coordinates> neighbor(const Coordinates& coordinates, Direction direction) const {
				Coordinates neighbor_coordinates = coordinates.neighbor(direction);
				if (not contains_vertically(neighbor_coordinates)) return nullptr;
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

		};

	}

}
