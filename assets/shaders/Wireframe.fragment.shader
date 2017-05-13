// shadertype=glsl
#version 150

// #define COLORED_EDGES
// #define COLORED_FACES

#include <shaders/Transparency.include>

uniform vec4 uFaceColor = vec4(vec3(0.0), 1.0);
uniform vec4 uEdgeColor = vec4(vec3(1.0), 1.0);

uniform float uBrightness = 1.0;
uniform float uTransparency = 0.0;

in VertexData {
	noperspective vec3 distance;
	vec4 color;
} Vertex;

out vec4 Output;

void main() {
	vec4 face_color = uFaceColor;
	vec4 edge_color = uEdgeColor;

	#ifdef COLORED_FACES
		face_color.rgb = Vertex.color.rgb;
	#endif

	#ifdef COLORED_EDGES
		edge_color.rgb = Vertex.color.rgb;
	#endif

	// determine fragment distance to closest edge
	float nearest = min(min(Vertex.distance[0], Vertex.distance[1]), Vertex.distance[2]);

	// blend between edge color and face color
	float transition = clamp(nearest * nearest * 0.1, 0.0, 1.0);
	Output = mix(edge_color, face_color, transition);

	Output.rgb *= uBrightness;
	Output.a *= transparency_to_alpha(uTransparency);
}
