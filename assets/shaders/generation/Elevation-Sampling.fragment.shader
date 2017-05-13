// shadertype=glsl
#version 330

#include <shaders/Mathematics.include>

#ifdef ORIGIN_UPPER_LEFT
	layout(origin_upper_left) in vec4 gl_FragCoord;
#endif

uniform sampler2D uElevationMap;

uniform float uElevationScale = 1.0;

in vec2 Texinates;

out vec4 Output; 

void main() {

	float elevation = texture(uElevationMap, Texinates).x;

	Output.r = to_signed_range(elevation) * uElevationScale;
	Output.a = 1.0;

}
