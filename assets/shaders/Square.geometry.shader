// shadertype=glsl
#version 330

// generic geometry shader to use in combination with the generic vertex shader
// used by sending a single point: glDrawArrays(GL_POINTS, 0, 1)
// produces a unit square geometry to use with a simulation fragment shader

// #define ORIGIN_UPPER_LEFT

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

out vec2 Texinates;

void main() {

	gl_Position = vec4(1.0, 1.0, 0.0, 1.0);
	#ifdef ORIGIN_UPPER_LEFT
		Texinates = vec2(1.0, 0.0);
	#else
		Texinates = vec2(1.0, 1.0);
	#endif
	EmitVertex();

	gl_Position = vec4(-1.0, 1.0, 0.0, 1.0);
	#ifdef ORIGIN_UPPER_LEFT
		Texinates = vec2(0.0, 0.0);
	#else
		Texinates = vec2(0.0, 1.0);
	#endif
	EmitVertex();

	gl_Position = vec4(1.0,-1.0, 0.0, 1.0);
	#ifdef ORIGIN_UPPER_LEFT
		Texinates = vec2(1.0, 1.0);
	#else
		Texinates = vec2(1.0, 0.0);
	#endif
	EmitVertex();

	gl_Position = vec4(-1.0,-1.0, 0.0, 1.0);
	#ifdef ORIGIN_UPPER_LEFT
		Texinates = vec2(0.0, 1.0);
	#else
		Texinates = vec2(0.0, 0.0);
	#endif
	EmitVertex();

	EndPrimitive();

}
