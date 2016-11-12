// shadertype=glsl
#version 330

#include <shaders/Mathematics.include>
#include <shaders/Noise.include>

#ifdef ORIGIN_UPPER_LEFT
	layout(origin_upper_left) in vec4 gl_FragCoord;
#endif

uniform sampler2D uElevationMap;
uniform sampler2D uTemperatureMap;
uniform sampler2D uHumidityMap;

in vec2 Texinates;

out vec4 Output; 

ivec2 texel;

void main() {

	texel = ivec2(gl_FragCoord);

	Output.rgb = vec3(0.5);
	Output.a = 1.0;
	
}
