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

in vec3 instancePosition;
in vec3 instanceColor;

out vec4 vPosition;
out vec3 vNormal;
out vec2 vTexCoord0;
out vec3 vColor;

void main() {
	vec4 position = ciPosition + vec4(instancePosition, 0);

	vPosition = ciModelView * position;
	vNormal = normalize(ciNormalMatrix * ciNormal);
	vTexCoord0 = ciTexCoord0;
	vColor = instanceColor;

	#ifdef HEIGHT_MAP
		float height = texture(uHeightMap, vTexCoord0).r;
		position.xyz += ciNormal * height * uHeightScale;
	#endif

	gl_Position = ciModelViewProjection * position;
}
