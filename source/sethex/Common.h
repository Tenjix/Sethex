#pragma once

#include <vector>
#include <unordered_map>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>

#include <utilities/Properties.h>
#include <utilities/Standard.h>

namespace sethex {

	using float2 = glm::vec2;
	using float3 = glm::vec3;
	using float4 = glm::vec4;
	using signed2 = glm::ivec2;
	using signed3 = glm::ivec3;
	using signed4 = glm::ivec4;
	using unsigned2 = glm::uvec2;
	using unsigned3 = glm::uvec3;
	using unsigned4 = glm::uvec4;
	using quaternion = glm::quat;
	using matrix = glm::mat4x4;

	using std::pair;
	using std::vector;
	using std::unordered_map;

	struct generic_zero {

		template <class Type>
		operator Type() {
			return Type(0);
		}

		template <class Type>
		bool operator==(Type value) {
			return value == Type(0);
		}

		template <class Type>
		bool operator!=(Type value) {
			return value != Type(0);
		}

	};

	template <class Type>
	bool operator==(Type value, generic_zero) {
		return value == Type(0);
	}
	template <class Type>
	bool operator!=(Type value, generic_zero) {
		return value != Type(0);
	}

	constexpr generic_zero zero;

}
