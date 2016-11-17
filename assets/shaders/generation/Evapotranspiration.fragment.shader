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

// calculates moisture based on circulation cells
float calculate_moisture() {
	float verticality = Texinates.y - uEquator;
	return -cos(verticality * 3 * Tau) * 0.5 + 0.5;
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
