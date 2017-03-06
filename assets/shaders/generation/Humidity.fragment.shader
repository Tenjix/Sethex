// shadertype=glsl
#version 330

#include <shaders/Mathematics.include>

#ifdef ORIGIN_UPPER_LEFT
	layout(origin_upper_left) in vec4 gl_FragCoord;
#endif

uniform sampler2D uElevationMap;
uniform sampler2D uCirculationMap;
uniform sampler2D uHumidityMap;

uniform float uSeaLevel = 0.0;
uniform float uIntensity = 1.0;
uniform float uOrograpicEffect = 1.0;
uniform uint uIteration = 1u;

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

void main() {

	ivec2 resolution = textureSize(uElevationMap, 0);
	ivec2 texel = ivec2(gl_FragCoord);

	float elevation = texture(uElevationMap, Texinates).r;
	float humidity = texture(uHumidityMap, Texinates).r;
	vec2 wind_direction = normalize(texture(uCirculationMap, Texinates).rg);
	float wind_speed = texture(uCirculationMap, Texinates).b;

	vec3 elevation_gradient = gradient(uElevationMap, texel, 5);
	float slope = 1.0 - dot(elevation_gradient, vec3(0,1,0));
	float orographic_effect = 1.0 - slope * uOrograpicEffect;

	bool land = elevation > uSeaLevel;
	float intensity = float(land) * uIntensity * 0.5;
	float scale = float(uIteration) * 0.01;

	// circulate humidity

	float inflow_humidity = texture(uHumidityMap, Texinates - wind_direction * wind_speed * scale).x;
	float outflow_humidity = texture(uHumidityMap, Texinates + wind_direction * wind_speed * scale).x;

	float inflow = max(inflow_humidity - humidity, 0.0);
	float outflow = max(humidity - outflow_humidity, 0.0) * uOrograpicEffect;
	humidity += inflow * intensity;
	humidity -= outflow * intensity;

	Output.r = humidity;
	Output.a = 1.0;
	
}
