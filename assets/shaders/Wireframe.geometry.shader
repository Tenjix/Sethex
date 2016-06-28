// shadertype=glsl
#version 150

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform ivec2 ciWindowSize;

out VertexData {
	noperspective vec3 distance;
	vec4 color;
} o;

void main() {

	// calculations for single-pass wireframe rendering

	// calculate triangle area (x2)

	vec2 p0 = ciWindowSize * gl_in[0].gl_Position.xy / gl_in[0].gl_Position.w;
	vec2 p1 = ciWindowSize * gl_in[1].gl_Position.xy / gl_in[1].gl_Position.w;
	vec2 p2 = ciWindowSize * gl_in[2].gl_Position.xy / gl_in[2].gl_Position.w;

	vec2 v0 = p2-p1;
	vec2 v1 = p2-p0;
	vec2 v2 = p1-p0;

	float area = abs(v1.x * v2.y - v1.y * v2.x);

	// calculate distance from center for each vertex (area (x2) / length of opposing edge)

	o.distance = vec3(area / length(v0), 0, 0);
	o.color = vec4(1.0, 0.0, 0.0, 1.0);
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();

	o.distance = vec3(0, area / length(v1), 0);
	o.color = vec4(0.0, 1.0, 0.0, 1.0);
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();

	o.distance = vec3(0, 0, area / length(v2));
	o.color = vec4(0.0, 0.0, 1.0, 1.0);
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();

	EndPrimitive();
}
