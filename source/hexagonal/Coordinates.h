#pragma once

#include <iostream>
#include <iomanip>

#include <hexagonal/Hexagonal.h>

#include <utilities/Properties.h>
#include <utilities/Exceptions.h>

namespace tenjix {

	namespace hexagonal {

		// Hexagonal Coordinates based on a homogeneous coordinate system with u + v + w = 0.
		// The u-axis points north-east, the v-axis points south and the w-axis points north-west.
		class Coordinates {

			int _u, _v;

			int get_u() const { return _u; }
			int get_v() const { return _v; }
			int get_w() const { return 0 - _u - _v; }

			void set_u(int u) { _u = u; }
			void set_v(int v) { _v = v; }
			void set_w(int w) { _u = 0 - _v - w; }

		public:

			static const Coordinates Origin;
			static const float2 Spacing;
			static const float2 Tilesize;

			ByValueProperty<int, Coordinates, &Coordinates::get_u, &Coordinates::set_u> u;
			ByValueProperty<int, Coordinates, &Coordinates::get_v, &Coordinates::set_v> v;
			ByValueProperty<int, Coordinates, &Coordinates::get_w, &Coordinates::set_w> w;

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
				this->u.owner = this;
				this->v.owner = this;
				this->w.owner = this;
			}

			// Creates coordinates by rounding from floating point values.
			Coordinates(double u, double v) : Coordinates(u, v, 0 - u - v) {}

			// Creates coordinates by validating the constraint u + v + w = 0.
			Coordinates(int u, int v, int w) : Coordinates(u, v) {
				if (u + v + w != 0) throw_runtime_exception("The sum u + v + w has to be 0.");
			}

			// Creates coordinates with constraint u + v + w = 0.
			Coordinates(int u, int v) : _u(u), _v(v) {
				this->u.owner = this;
				this->v.owner = this;
				this->w.owner = this;
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

			// Shifts these coordinates in the given heading by the given distance.
			Coordinates& shift(Heading direction, int distance = 1) {
				switch (direction) {
					case Heading::Northward:
						_u += distance;
						_v -= 2 * distance;
						break;
					case Heading::NorthEastward:
						_u += 2 * distance;
						_v -= 2 * distance;
						break;
					case Heading::EastNorthward:
						_u += 2 * distance;
						_v -= distance;
						break;
					case Heading::Eastward:
						_u += 2 * distance;
						break;
					case Heading::EastSouthward:
						_u += distance;
						_v += distance;
						break;
					case Heading::SouthEastward:
						_v -= 2 * distance;
						break;
					case Heading::Southward:
						_u -= distance;
						_v += 2 * distance;
						break;
					case Heading::SouthWestward:
						_u -= 2 * distance;
						_v += 2 * distance;
						break;
					case Heading::WestSouthward:
						_u -= 2 * distance;
						_v += distance;
						break;
					case Heading::Westward:
						_u -= 2 * distance;
						break;
					case Heading::WestNorthward:
						_u -= distance;
						_v -= distance;
						break;
					case Heading::NorthWestward:
						_v -= 2 * distance;
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

			unsigned distance(const Coordinates& coordinates) const {
				return distance(*this, coordinates);
			}

			unsigned magnitude() const {
				return magnitude(*this);
			}

			String to_string(unsigned spacing = 3) const;

			operator String() const { return to_string(); }

			float3 to_floats() const { return float3(_u, _v, w); }

			operator float3() const { return to_floats(); }

			// Convertes these hexagonal coordinates to cartesian coordinates.
			float2 to_cartesian() const {
				return float2(static_cast<float>(d::Sqrt_3 * (_u + _v / 2.0)), static_cast<float>(3.0 / 2.0 * _v));
			}

			// Convertes these hexagonal coordinates to a world position in the y=0 plane.
			float3 to_position() const {
				float2 cartesian = to_cartesian();
				return float3(cartesian.x, 0.0f, cartesian.y);
			}

			// Calculates the hexagonal coordinates of the given cartesian coordinates.
			static Coordinates of(float x, float y) {
				return Coordinates(x * d::Sqrt_3 / 3.0 - y / 3.0, 2.0 / 3.0 * y);
			}

			// Calculates the hexagonal coordinates of the given cartesian coordinates.
			static Coordinates of(float2 cartesian) {
				return of(cartesian.x, cartesian.y);
			}

			// Calculates the hexagonal coordinates of the given world position by projecting it to the y=0 plane.
			static Coordinates of(float3 position) {
				return of(position.x, position.z);
			}

			// Calculates hexagonal coordinates in a line between "begin" and "end". "supercover"  
			static Lot<Coordinates> line(const Coordinates& begin, const Coordinates& end, bool supercover = false);

			// Calculates hexagonal coordinates in a ring pattern with "radius" around "center".
			static Lot<Coordinates> ring(unsigned radius, const Coordinates& center = Coordinates::Origin);

			// Calculates hexagonal coordinates in a spiral pattern with "radius" around "center".
			static Lot<Coordinates> spiral(unsigned radius, const Coordinates& center = Coordinates::Origin);

			// Calculates hexagonal coordinates in a rectangle pattern with "width" and "height" at "origin".
			static Lot<Coordinates> rectangle(unsigned width, unsigned height, const Coordinates& origin = Coordinates::Origin, bool centered = true);

			// Calculates the hexagonal distance between two coordinates. 
			static unsigned distance(const Coordinates& one, const Coordinates& two) {
				return (abs(one.u - two.u) + abs(one.v - two.v) + abs(one.w - two.w)) / 2;
			}

			// Calculates the hexagonal distance of the given coordinates to the origin. 
			static unsigned magnitude(const Coordinates& coordinates) {
				return distance(coordinates, Origin);
			}

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
	struct hash<tenjix::hexagonal::Coordinates> {
		size_t operator()(const tenjix::hexagonal::Coordinates& coordinates) const noexcept {
			return tenjix::hash_combined(coordinates.u(), coordinates.v());
		}
	};

}
