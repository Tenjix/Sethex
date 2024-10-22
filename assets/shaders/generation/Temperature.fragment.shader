// shadertype=glsl
#version 330

#include <shaders/Mathematics.include>
#include <shaders/Noise.include>

#ifdef ORIGIN_UPPER_LEFT
	layout(origin_upper_left) in vec4 gl_FragCoord;
#endif

uniform sampler2D uElevationMap;

uniform float uSeaLevel = 0.0;
uniform float uEquator = 0.0;
uniform float uLapseRate = 10.0 / 70.0;
uniform float uElevationScale = 10.0;

in vec2 Texinates;

out vec4 Output;

// calcualtes normalized distance to equator
float calculate_distance_to_equator() {
	float latitude = to_signed_range(Texinates.y);
	float delta = latitude - uEquator;
	float range = 1.0 - sign(delta) * uEquator;
	return abs(delta) / range;
}

void main() {

	float elevation = texture(uElevationMap, Texinates).r;
	float distance_to_equator = calculate_distance_to_equator();
	float distance_to_equator_squared = distance_to_equator * distance_to_equator;
	
	// float percentage_elevation_above_sealevel = max(elevation - uSeaLevel, 0.0f) / (1.0f - uSeaLevel);
	float percentage_temperature_decline_per_km = uLapseRate;
	float elevation_above_sealevel_in_km = (elevation - uSeaLevel) * uElevationScale;
	float elevation_based_temerature_decline = percentage_temperature_decline_per_km * elevation_above_sealevel_in_km;

	vec2 position = gl_FragCoord.xy;
	float temperature_noise = simplex_noise(position.xy * 0.005);

	float temperature = 1.0f - distance_to_equator_squared;
	// temperature = 1.0f - (distance_to_equator_squared + distance_to_equator) * 0.5;
	// temperature = mix(temperature, temperature_noise, distance_to_equator * 0.1);
	temperature -= elevation > uSeaLevel ? elevation_based_temerature_decline : 0.1;

	Output.r = temperature;
	Output.a = 1.0;

}
