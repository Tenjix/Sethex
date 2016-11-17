// shadertype=glsl
#version 330

#include <shaders/Mathematics.include>

#ifdef ORIGIN_UPPER_LEFT
	layout(origin_upper_left) in vec4 gl_FragCoord;
#endif

uniform sampler2D uElevationMap;
uniform sampler2D uTemperatureMap;
uniform sampler2D uCirculationMap;
uniform sampler2D uHumidityMap;

uniform uvec2 uResolution;
uniform float uSeaLevel = 0.0;
uniform float uOrograpicEffect = 1.0;

in vec2 Texinates;

out vec4 Output; 

ivec2 limit(ivec2 texel, ivec2 map_size) {
	texel.x = project(texel.x, 0, map_size.x - 1);
	texel.y = clamp(texel.y, 0, map_size.y - 1);
	return texel;
}

vec4 read(sampler2D sampler, ivec2 texel) {
	ivec2 resolution = textureSize(sampler, 0);
	return texelFetch(sampler, limit(texel, resolution), 0);
}

vec3 gradient(sampler2D sampler, ivec2 texel, int delta) {
	return normalize(vec3(
		read(sampler, texel + ivec2(delta, 0)).r - read(sampler, texel - ivec2(delta, 0)).r,
		0.01 * delta,
		read(sampler, texel + ivec2(0, delta)).r - read(sampler, texel - ivec2(0, delta)).r
	));
}

float global_precipitation(ivec2 texel) {
	float verticality = float(texel.y) / uResolution.y;
	return 1.0 - (cos(verticality * 6 * Tau_Half) + 1.0) / 2.0;
}

void main() {

	ivec2 texel = ivec2(gl_FragCoord);

	float elevation = read(uElevationMap, texel).r;
	float temperature = read(uTemperatureMap, texel).r;
	float humidity = read(uHumidityMap, texel).r;
	vec2 wind_direction = normalize(texture(uCirculationMap, Texinates).rg);
	float wind_speed = texture(uCirculationMap, Texinates).b;
	bool land = elevation > uSeaLevel;

	vec3 elevation_gradient = gradient(uElevationMap, texel, 5);
	float slope = 1.0 - dot(elevation_gradient, vec3(0,1,0));
	// vec3 temperature_gradient = gradient(uTemperatureMap, texel, 10);
	// float temperature_slope = dot(temperature_gradient, vec3(0,1,0));

	float uphill = max(dot(wind_direction, -elevation_gradient.xz), 0.0);
	float orographic_effect = land? uphill * slope * uOrograpicEffect : 0.0;

	float precipitation = global_precipitation(texel) * 0.5;
	precipitation += humidity * (temperature + orographic_effect);
	precipitation = min(precipitation, 1.0);

	Output.r = precipitation;
	Output.a = 1.0;

}
