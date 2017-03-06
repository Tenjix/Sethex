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

uniform float uSeaLevel = 0.0;
uniform float uEquator = 0.0;
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

// calcualtes normalized distance to equator
float calculate_distance_to_equator() {
	float latitude = to_signed_range(Texinates.y);
	float delta = latitude - uEquator;
	float range = 1.0 - sign(delta) * uEquator;
	return abs(delta) / range;
}

// calculates precipitation based on circulation cells
float base_precipitation() {
	float verticality = to_unsigned_range(calculate_distance_to_equator());
	return to_unsigned_range(-cos(verticality * 3 * Tau));
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

	float base = base_precipitation();
	float orographic = humidity * (temperature + orographic_effect);
	float precipitation = min(0.5 * base + 1.25 * orographic, 1.0);

	Output.r = precipitation;
	Output.a = 1.0;

}
