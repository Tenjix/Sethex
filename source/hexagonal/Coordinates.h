#pragma once

#include <iostream>
#include <iomanip>

#include <hexagonal/Hexagonal.h>

#include <utilities/Properties.h>

namespace tenjix {

	namespace hexagonal {

		// Hexagonal Coordinates based on a homogeneous coordinate system with u + v + w = 0.
		// The u-axis points north-east, the v-axis points south and the w-axis points north-west.
		class Coordinates {

			static const Coordinates Direction_Coordinates[6];
			static const Coordinates Heading_Coordinates[12];

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

			ByValueProperty<int, Coordinates, &Coordinates::get_u, &Coordinates::set_u> u;
			ByValueProperty<int, Coordinates, &Coordinates::get_v, &Coordinates::set_v> v;
			ByValueProperty<int, Coordinates, &Coordinates::get_w, &Coordinates::set_w> w;

			// Creates coordinates by rounding from floating point values. The value with the biggest deviation from an integer will be adjusted to satisfy u + v + w = 0.
			Coordinates(double u, double v, double w) {
				if (u + v + w != 0.0) throw_runtime_exception("The sum u + v + w has to be 0.");
				double ru = round(u);
				double rv = round(v);
				double rw = round(w);
				double du = abs(u - ru);
				double dv = abs(v - rv);
				double dw = abs(w - rw);
				if (du > dv and du > dw) ru = 0 - rv - rw;
				else if (dv > dw) rv = 0 - ru - rw;
				_u = static_cast<int>(ru);
				_v = static_cast<int>(rv);
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

			Coordinates copy() const {
				return Coordinates(_u, _v);
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
						throw_runtime_exception();
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
						throw_runtime_exception();
				}
				return *this;
			}

			// Rotates these coordinates by 60 degree per iteration in the given direction.
			Coordinates& rotate(int iterations = 1, Rotating rotating = Rotating::Clockwise) {
				bool positive_iterations = sign(iterations) == 1;
				bool clockwise = (rotating == Rotating::Clockwise) ? positive_iterations : not positive_iterations;
				for (int i = 0; i < abs(iterations); i++) {
					int _w = w;
					if (clockwise) {
						_u = -_v;
						_v = -_w;
					} else {
						_u = -_w;
						_v = -_u;
					}
				}
				return *this;
			}

			Coordinates neighbor(Direction direction) const {
				return copy().shift(direction, 1);
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

			Coordinates operator+(Direction direction) const { return copy().shift(direction); }
			Coordinates operator-(Direction direction) const { return copy().shift(direction, -1); }
			Coordinates& operator+=(Direction direction) { return shift(direction); }
			Coordinates& operator-=(Direction direction) { return shift(direction, -1); }

			Coordinates operator+(Heading heading) const { return copy().shift(heading); }
			Coordinates operator-(Heading heading) const { return copy().shift(heading, -1); }
			Coordinates& operator+=(Heading heading) { return shift(heading); }
			Coordinates& operator-=(Heading heading) { return shift(heading, -1); }

			bool operator==(const Coordinates& other) const {
				return _u == other._u and _v == other._v;
			}

			bool operator!=(const Coordinates& other) const {
				return not operator==(other);
			}

			// Calculates the hexagonal distance to the origin. 
			unsigned magnitude() const {
				return (abs(u) + abs(v) + abs(w)) / 2;
			}

			// Returns the coordinates' component for a given axis.
			int component(Axis axis) const {
				switch (axis) {
					case Axis::U: return u;
					case Axis::V: return v;
					case Axis::W: return w;
					default: throw_runtime_exception();
				}
			}

			// Determines the axis of the coordinates' dominant component.
			Axis dominant_axis() const {
				int au = abs(u);
				int av = abs(v);
				int aw = abs(w);
				if (au >= av and au >= aw) return Axis::U;
				if (av >= aw) return Axis::V;
				return Axis::W;
			}

			// Determines the general heading from the origin to these coordinates.
			Heading general_heading() const {
				runtime_assert(magnitude() != 0, "Coordinates equal to the origin have no general heading.");
				auto axis = dominant_axis();
				return heading_of(axis, component(axis) >= 0);
			}

			// Determines the general direction from the origin to these coordinates.
			Direction general_direction() const {
				return static_cast<Direction>(static_cast<uint8>(general_heading()) / 2);
			}

			String to_string(unsigned spacing = 3) const;

			operator String() const { return to_string(); }

			float3 to_floats() const { return float3(_u, _v, w); }

			operator float3() const { return to_floats(); }

			// Convertes these hexagonal coordinates to offset coordinates. 
			float2 to_offset(Handedness handedness = Handedness::Right) const {
				auto v = handedness == Handedness::Right ? -_v : _v;
				return float2(_u + 0.5f * _v, v);
			}

			// Convertes these hexagonal coordinates to cartesian coordinates.
			float2 to_cartesian(Handedness handedness = Handedness::Right) const {
				auto offset = to_offset(handedness);
				return float2(f::Sqrt_3 * offset.x, 1.5f * offset.y);
			}

			// Convertes these hexagonal coordinates to a world position in the y=0 plane.
			float3 to_position(Handedness handedness = Handedness::Left) const {
				float2 cartesian = to_cartesian(handedness);
				return float3(cartesian.x, 0.0f, cartesian.y);
			}

			// Calculates the hexagonal coordinates of the given cartesian coordinates.
			static Coordinates of(float x, float y, Handedness handedness = Handedness::Right) {
				y = handedness == Handedness::Right ? -y : y;
				return Coordinates(d::One_Third * (d::Sqrt_3 * x - y), d::Two_Thirds * y);
			}

			// Calculates the hexagonal coordinates of the given cartesian coordinates.
			static Coordinates of(float2 cartesian, Handedness handedness = Handedness::Right) {
				return of(cartesian.x, cartesian.y, handedness);
			}

			// Calculates the hexagonal coordinates of the given world position by projecting it to the y=0 plane.
			static Coordinates of(float3 position, Handedness handedness = Handedness::Left) {
				return of(position.x, position.z, handedness);
			}

			// Returns directional coordinates for the given direction.
			static const Coordinates& going(Direction direction) {
				uint8 index = static_cast<uint8>(direction);
				return Direction_Coordinates[index];
			}

			// Returns directional coordinates for the given heading.
			static const Coordinates& heading(Heading heading) {
				uint8 index = static_cast<uint8>(heading);
				return Heading_Coordinates[index];
			}

			// Calculates hexagonal coordinates in a line between "begin" and "end". "supercover"  
			static Lot<Coordinates> line(const Coordinates& begin, const Coordinates& end, bool supercover = false, bool edgecover = true);

			// Calculates hexagonal coordinates in a ring pattern with "radius" around "center".
			static Lot<Coordinates> ring(unsigned radius, const Coordinates& center = Coordinates::Origin);

			// Calculates hexagonal coordinates in a spiral pattern with "radius" around "center".
			static Lot<Coordinates> spiral(unsigned radius, const Coordinates& center = Coordinates::Origin);

			// Calculates hexagonal coordinates in a rectangle pattern with "width" and "height" at "origin".
			static Lot<Coordinates> rectangle(unsigned width, unsigned height, const Coordinates& origin = Coordinates::Origin, bool centered = true);

		};

		// Calculates the hexagonal distance between two coordinates. 
		inline unsigned distance(const Coordinates& one, const Coordinates& two) {
			return (one - two).magnitude();
		}

		inline Coordinates operator*(Direction direction, int distance) {
			return Coordinates().shift(direction, distance);
		}

		inline std::ostream& append(std::ostream& output, const Coordinates& coordinates, unsigned spacing = 4) {
			auto width = std::setw(spacing);
			output << "(" << width << coordinates.u() << ", " << width << coordinates.v() << ", " << width << coordinates.w() << ")";
			return output;
		}

		inline std::ostream& operator<<(std::ostream& output, const Coordinates& coordinates) {
			return append(output, coordinates);
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
