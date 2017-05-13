// shadertype=glsl
#version 330

#include <shaders/Mathematics.include>
#include <shaders/Noise.include>

#ifdef ORIGIN_UPPER_LEFT
	layout(origin_upper_left) in vec4 gl_FragCoord;
#endif

uniform sampler2D uElevationMap;
uniform sampler2D uTemperatureMap;
uniform sampler2D uPrecipitationMap;

uniform float uSeaLevel = 0.0;

in vec2 Texinates;

out vec4 Output; 

void main() {

	float elevation = texture(uElevationMap, Texinates).x;

	Output.rgb = vec3(temperature);
	Output.a = 1.0;

}
