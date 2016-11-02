#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>

#include <utilities/Properties.h>
#include <utilities/Standard.h>

namespace tenjix {

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

	}

	#ifndef SETHEX_NO_NAMESPACE_ALIAS
	namespace sh = sethex;
	#endif

}

