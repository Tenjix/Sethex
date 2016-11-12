// shadertype=glsl
#version 330

#include <shaders/Mathematics.include>
#include <shaders/Noise.include>

#ifdef ORIGIN_UPPER_LEFT
	layout(origin_upper_left) in vec4 gl_FragCoord;
#endif

uniform sampler2D uElevationMap;
uniform sampler2D uTemperatureMap;

uniform float uSeaLevel = 0.0;
uniform float uEvaporation = 1.0;
uniform float uTranspiration = 0.2;

in vec2 Texinates;

out vec4 Output; 

void main() {

	float sea_level = uSeaLevel * 0.5 + 0.5;
	float elevation = texture(uElevationMap, Texinates).x;
	float temperature = texture(uTemperatureMap, Texinates).x;

	bool water = elevation <= sea_level;
	float evapotranspiration = water ? uEvaporation : uTranspiration;
	float humidity = evapotranspiration * mix(0.5, 1.0, temperature);

	Output.rgb = vec3(humidity);
	Output.a = 1.0;

}
