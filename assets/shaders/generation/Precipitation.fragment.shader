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
uniform float uIntensity = 1.0;
uniform float uCirculation = 0.5;
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
float estimate_base_precipitation() {
	float verticality = to_unsigned_range(calculate_distance_to_equator());
	return to_unsigned_range(-cos(verticality * 3 * Tau));
}

float calculate_orograpic_effect(vec3 elevation_gradient, vec2 wind_direction, bool land) {
	float slope = land? 1.0 - dot(elevation_gradient, vec3(0,1,0)) : 0.0;
	float uphill = max(dot(wind_direction, -elevation_gradient.xz), 0.0);
	return uphill * slope * uOrograpicEffect;
}

void main() {

	ivec2 texel = ivec2(gl_FragCoord);

	float elevation = read(uElevationMap, texel).r;
	vec3 elevation_gradient = gradient(uElevationMap, texel, 5);
	float temperature = read(uTemperatureMap, texel).r;
	float humidity = read(uHumidityMap, texel).r;
	vec2 wind_direction = normalize(texture(uCirculationMap, Texinates).rg);
	float wind_speed = texture(uCirculationMap, Texinates).b;
	bool land = elevation > uSeaLevel;

	float orographic_effect = calculate_orograpic_effect(elevation_gradient, wind_direction, land);

	float base = estimate_base_precipitation();
	float general = humidity * temperature;
	float orographic = humidity * orographic_effect;
	float estimated = (1.0 - uCirculation) * base;
	float simulated = (2.0 * uCirculation) * (general + orographic);
	float precipitation = float(land) * uIntensity * (estimated + simulated);

	Output.r = precipitation;
	Output.a = 1.0;

}
