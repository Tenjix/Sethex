// shadertype=glsl
#version 150

// #undef HEIGHT_MAP

uniform sampler2D uHeightMap;

uniform float uHeightScale = 1.0;

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelView;
uniform mat3 ciNormalMatrix;

in vec4 ciPosition;
in vec3 ciNormal;
in vec2 ciTexCoord0;

out vec4 vPosition;
out vec3 vNormal;
out vec2 vTexCoord0;

void main() {
	vPosition = ciModelView * ciPosition;
	vNormal = normalize(ciNormalMatrix * ciNormal);
	vTexCoord0 = ciTexCoord0;

	vec4 position = ciPosition;

	#ifdef HEIGHT_MAP
		float height = texture(uHeightMap, vTexCoord0).r;
		position.xyz += ciNormal * height * uHeightScale;
	#endif

	// gl_Position = ciModelViewProjection * ciPosition;
	gl_Position = ciModelViewProjection * position;
}
