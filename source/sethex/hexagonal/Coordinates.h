#pragma once

#include <cmath>
#include <iostream>

#include <sethex/Common.h>

#include <utilities/Exceptions.h>

namespace sethex {

	namespace hexagonal {

		enum class Direction : uint8 {
			NorthEast, East, SouthEast, SouthWest, West, NorthWest
		};

		// Hexagonal Coordinates based on a homogeneous coordinate system with u + v + w = 0.
		// The u-axis points north-east, the v-axis points south and the w-axis points north-west.
		class Coordinates {

			int _u, _v;

			int get_u() const { return _u; }
			int get_v() const { return _v; }
			int get_w() const { return 0 - _u - _v; }

		public:

			static const Coordinates Origin;

			ReadonlyValueProperty<int, Coordinates, &Coordinates::get_u> u;
			ReadonlyValueProperty<int, Coordinates, &Coordinates::get_v> v;
			ReadonlyValueProperty<int, Coordinates, &Coordinates::get_w> w;

			// Creates coordinates by rounding from floating point values. The value with the biggest deviation from an integer will be adjusted to satisfy u + v + w = 0.
			Coordinates(double u, double v, double w) {
				if (u + v + w != 0.0) throw_runtime_exception("The sum u + v + w has to be 0.");
				double rounded_u = round(u);
				double rounded_v = round(v);
				double rounded_w = round(w);
				double delta_u = abs(u - rounded_u);
				double delta_v = abs(v - rounded_v);
				double delta_w = abs(w - rounded_w);
				if (delta_u > delta_v && delta_u > delta_w) rounded_u = 0 - rounded_v - rounded_w;
				else if (delta_v > delta_w) rounded_v = 0 - rounded_u - rounded_w;
				_u = static_cast<int>(rounded_u);
				_v = static_cast<int>(rounded_v);
				this->u._property_owner(this);
				this->v._property_owner(this);
				this->w._property_owner(this);
			}

			// Creates coordinates by rounding from floating point values.
			Coordinates(double u, double v) : Coordinates(u, v, 0 - u - v) {}

			// Creates coordinates by validating the constraint u + v + w = 0.
			Coordinates(int u, int v, int w) : Coordinates(u, v) {
				if (u + v + w != 0) throw_runtime_exception("The sum u + v + w has to be 0.");
			}

			// Creates coordinates with constraint u + v + w = 0.
			Coordinates(int u, int v) : _u(u), _v(v) {
				this->u._property_owner(this);
				this->v._property_owner(this);
				this->w._property_owner(this);
			}

			// Creates coordinates of the origin.
			Coordinates() : Coordinates(0, 0) {}

			~Coordinates() {}

			Coordinates(const Coordinates& other) : Coordinates(other._u, other._v) {}

			Coordinates& operator=(const Coordinates& other) {
				_u = other._u;
				_v = other._v;
				return *this;
			}

			// Shifts these coordinates in the given direction by the given distance.
			Coordinates& shift(Direction direction, int distance = 1) {
				switch (direction) {
					case Direction::NorthEast:
						_u += distance;
						_v -= distance;
						break;
					case Direction::East:
						_u += distance;
						break;
					case Direction::SouthEast:
						_v += distance;
						break;
					case Direction::SouthWest:
						_u -= distance;
						_v += distance;
						break;
					case Direction::West:
						_u -= distance;
						break;
					case Direction::NorthWest:
						_v -= distance;
						break;
					default:
						throw_runtime_exception("unknown direction");
						break;
				}
				return *this;
			}

			Coordinates neighbor(Direction direction) const {
				Coordinates neighbor(*this);
				neighbor.shift(direction, 1);
				return neighbor;
			}

			Coordinates operator+(const Coordinates& other) const {
				return Coordinates(_u + other._u, _v + other._v);
			}

			Coordinates operator-(const Coordinates& other) const {
				return Coordinates(_u - other._u, _v - other._v);
			}

			Coordinates& operator+=(const Coordinates& other) {
				_u += other._u;
				_v += other._v;
				return *this;
			}

			Coordinates& operator-=(const Coordinates& other) {
				_u -= other._u;
				_v -= other._v;
				return *this;
			}

			bool operator==(const Coordinates& other) const {
				return _u == other._u && _v == other._v;
			}

			bool operator!=(const Coordinates& other) const {
				return not operator==(other);
			}

			String to_string(unsigned spacing = 3) const;

			operator String() const { return to_string(); }

			float2 to_floats() const { return float2(_u, _v); }

			operator float2() const { return to_floats(); }

			// Convertes these hexagonal coordinates to cartesian coordinates.
			float2 to_cartesian() const {
				return float2(static_cast<float>(Sqrt_3 * (_u + _v / 2.0)), static_cast<float>(3.0 / 2.0 * _v));
			}

			// Convertes these hexagonal coordinates to a world position in the y=0 plane.
			float3 to_position() const {
				float2 cartesian = to_cartesian();
				return float3(cartesian.x, 0.0f, cartesian.y);
			}

			// Calculates the hexagonal coordinates of the given cartesian coordinates.
			static Coordinates of(float x, float y) {
				return Coordinates(x * Sqrt_3 / 3.0 - y / 3.0, 2.0 / 3.0 * y);
			}

			// Calculates the hexagonal coordinates of the given cartesian coordinates.
			static Coordinates of(float2 cartesian) {
				return of(cartesian.x, cartesian.y);
			}

			// Calculates the hexagonal coordinates of the given world position by projecting it to the y=0 plane.
			static Coordinates of(float3 position) {
				return of(position.x, position.z);
			}

			// Calculates hexagonal coordinates in a ring pattern with "radius" around "center".
			static vector<Coordinates> ring(unsigned radius, const Coordinates& center = Coordinates::Origin);

			// Calculates hexagonal coordinates in a spiral pattern with "radius" around "center".
			static vector<Coordinates> spiral(unsigned radius, const Coordinates& center = Coordinates::Origin);

			// Calculates hexagonal coordinates in a rectangle pattern with "width" and "height" at "origin".
			static vector<Coordinates> rectangle(unsigned width, unsigned height, const Coordinates& origin = Coordinates::Origin, bool centered = true);

		};

		inline std::ostream& append(std::ostream& output, const Coordinates& coordinates, unsigned spacing = 4) {
			auto width = std::setw(spacing);
			output << "(" << width << coordinates.u() << ", " << width << coordinates.v() << ", " << width << coordinates.w() << ")";
			return output;
		}

		inline std::ostream& operator<<(std::ostream& output, const Coordinates& coordinates) {
			return append(output, coordinates);
		}

		inline Coordinates operator*(Direction direction, int distance) {
			return Coordinates().shift(direction, distance);
		}

	}

}

namespace std {

	template<>
	struct hash<sethex::hexagonal::Coordinates> {
		size_t operator()(const sethex::hexagonal::Coordinates& coordinates) const noexcept {
			return hash_combined(coordinates.u(), coordinates.v());
		}
	};

}