// shadertype=glsl
#version 330

#include <shaders/Mathematics.include>
#include <shaders/Noise.include>

#ifdef ORIGIN_UPPER_LEFT
	layout(origin_upper_left) in vec4 gl_FragCoord;
#endif

uniform sampler2D uElevationMap;

uniform uvec2 uResolution;
uniform float uSeaLevel = 0.5;

in vec2 Texinates;

out vec4 Output; 

ivec2 texel;

void main() {

	texel = ivec2(gl_FragCoord);

	float distance_to_equator = abs(2.0 * (Texinates.y - 0.5));

	float elevation = texture(uElevationMap, texinates).x;
	// float percentage_elevation_above_sealevel = max(elevation - uSeaLevel, 0.0f) / (1.0f - uSeaLevel);
	float elevation_above_sealevel_in_km = max(elevation - uSeaLevel, 0.0) * 10.0;
	float elevation_based_temerature_decline = lapse_rate * elevation_above_sealevel_in_km / 70.0;

	vec2 position = gl_FragCoord.xy;
	float temperature_noise = simplex_noise(position * 0.01);

	float temperature = 1.0f - distance_to_equator * distance_to_equator;
	// temperature = mix(temperature, temperature_noise, 0.1f);
	temperature -= elevation_based_temerature_decline;

	Output.rgb = vec3(temperature);
	Output.a = 1.0;

}
