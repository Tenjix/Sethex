// shadertype=glsl
#version 150

// #define COLORED_EDGES
// #define COLORED_FACES

#include <shaders/Transparency.h>

uniform vec4 uFaceColor = vec4(vec3(0.0), 1.0);
uniform vec4 uEdgeColor = vec4(vec3(1.0), 1.0);

uniform float uBrightness = 1.0;
uniform float uTransparency = 0.0;

in VertexData {
	noperspective vec3 distance;
	vec4 color;
} i;

out vec4 oColor;

void main() {
	vec4 face_color = uFaceColor;
	vec4 edge_color = uEdgeColor;

	#ifdef COLORED_FACES
		face_color.rgb = i.color.rgb;
	#endif

	#ifdef COLORED_EDGES
		edge_color.rgb = i.color.rgb;
	#endif

	// determine fragment distance to closest edge
	float nearest = min(min(i.distance[0], i.distance[1]), i.distance[2]);

	// blend between edge color and face color
	float transition = clamp(nearest * nearest * 0.1, 0.0, 1.0);
	oColor = mix(edge_color, face_color, transition);

	oColor.rgb *= uBrightness;
	oColor.a *= transparency_to_alpha(uTransparency);
}
