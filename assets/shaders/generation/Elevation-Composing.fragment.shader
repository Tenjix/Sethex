// shadertype=glsl
#version 330

#include <shaders/Mathematics.include>

#ifdef ORIGIN_UPPER_LEFT
	layout(origin_upper_left) in vec4 gl_FragCoord;
#endif

uniform sampler2D uBathymetryMap;
uniform sampler2D uTopographyMap;

uniform float uBathymetryScale = 1.0;
uniform float uTopographyScale = 1.0;

in vec2 Texinates;

out vec4 Output; 

void main() {

	float bathymetry = texture(uBathymetryMap, Texinates).x;
	float topography = texture(uTopographyMap, Texinates).x;

	float elevation = (-1.0 + bathymetry) * uBathymetryScale + topography * uTopographyScale;

	Output.r = elevation;
	Output.a = 1.0;

}
