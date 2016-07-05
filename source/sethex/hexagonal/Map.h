#pragma once

#include <sethex/hexagonal/Coordinates.h>

#include <utilities/Optional.h>

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

		public:

			const vector<Coordinates>& coordinates() const {
				return content;
			}

			Map() : width(), height(), u_begin(), u_end(), v_begin(), v_end() {}

			Map(unsigned width, unsigned height) : width(width), height(height) {
				u_begin = -static_cast<int>(width / 2);
				u_end = u_begin + width;
				v_begin = -static_cast<int>(height / 2);
				v_end = v_begin + height;
				content.reserve(width * height);
				for (int v = v_begin; v < v_end; v++) {
					int shift = (v % 2 == -1) ? (v / 2 - 1) : (v / 2);
					for (int u = u_begin - shift; u < u_end - shift; u++) {
						content.push_back(Coordinates(u, v));
					}
				}
			}

			bool within_horizontal_limits(Coordinates coordinates) {
				int shift = coordinates.v / 2 - (coordinates.v % 2 == -1);
				return coordinates.u >= u_begin - shift and coordinates.u < u_end - shift;
			}

			bool within_vertical_limits(Coordinates coordinates) {
				return coordinates.v >= v_begin and coordinates.v < v_end;
			}

			bool contains(Coordinates coordinates) {
				return within_vertical_limits(coordinates) and within_horizontal_limits(coordinates);
			}

			Optional<Coordinates> neighbor(Coordinates coordinates, Direction direction) {
				return Optional<Coordinates>();
			}

		};

	}

}