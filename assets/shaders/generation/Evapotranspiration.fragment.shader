// shadertype=glsl
#version 330

#include <shaders/Mathematics.include>

#ifdef ORIGIN_UPPER_LEFT
	layout(origin_upper_left) in vec4 gl_FragCoord;
#endif

uniform sampler2D uElevationMap;
uniform sampler2D uTemperatureMap;
uniform sampler2D uCirculationMap;

uniform float uSeaLevel = 0.0;
uniform float uEquator = 0.0;
uniform float uEvaporation = 1.0;
uniform float uTranspiration = 0.5;

in vec2 Texinates;

out vec4 Output; 

// calcualtes normalized distance to equator
float calculate_distance_to_equator() {
	float latitude = to_signed_range(Texinates.y);
	float delta = latitude - uEquator;
	float range = 1.0 - sign(delta) * uEquator;
	return abs(delta) / range;
}

// calculates moisture based on circulation cells
float calculate_moisture() {
	float verticality = to_unsigned_range(calculate_distance_to_equator());
	return to_unsigned_range(-cos(verticality * 3 * Tau));
}

void main() {

	float elevation = texture(uElevationMap, Texinates).r;
	float temperature = texture(uTemperatureMap, Texinates).r;
	float wind_speed = texture(uCirculationMap, Texinates).b;
	float moisture = calculate_moisture();

	bool water = elevation <= uSeaLevel;
	float evapotranspiration = water ? uEvaporation : uTranspiration;
	float humidity = evapotranspiration * mix(0.5, 1.0, moisture) * mix(0.5, 1.0, temperature) * mix(0.75, 1.0, wind_speed);

	Output.r = humidity;
	Output.a = 1.0;

}
