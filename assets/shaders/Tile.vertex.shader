// shadertype=glsl
#version 150

#include <shaders/Texinates.h>

uniform sampler2D uHeightMap;

uniform float uHeightScale = 1.0;

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelView;
uniform mat3 ciNormalMatrix;

uniform vec3 uLightPosition = vec3(0, 1, 0);

uniform float uLightIntensity = 1.0;
uniform float uNormalIntensity = 1.0;

uniform vec3 uDiffuseColor = vec3(1.0);
uniform vec3 uSpecularColor = vec3(1.0);
uniform vec3 uEmissiveColor = vec3(1.0);
uniform vec3 uOverlayColor = vec3(1.0);

uniform float uAmbience = 0.1;
uniform float uSpecularity = 0.0;
uniform float uLuminosity = 0.0;
uniform float uRoughness = 0.1;
uniform float uTransparency = 0.0;

uniform vec2 uTextureScale = vec2(1.0);
uniform vec2 uTextureShift = vec2(0.0);
uniform mat2x2 uTextureRotation = mat2x2(1.0);

uniform vec2 uOverlayScale = vec2(1.0);
uniform vec2 uOverlayShift = vec2(0.0);
uniform mat2x2 uOverlayRotation = mat2x2(1.0);

in vec4 ciPosition;
in vec3 ciNormal;
in vec3 ciColor;
in vec2 ciTexCoord0;

in vec3 instance_position;
in vec3 instance_color;

flat out vec3 diffuse_color;
flat out vec3 specular_color;
flat out vec3 emissive_color;
flat out vec3 overlay_color;
flat out vec3 light_position;

flat out float light_intensity;
flat out float normal_intensity;

flat out float ambience;
flat out float specularity;
flat out float luminosity;
flat out float roughness;
flat out float transparency;
flat out float alpha;

out vec3 position;
out vec3 normal;
out vec3 color;
out vec2 texinates;
out vec2 overlay_texinates;

void main() {
	vec4 vertex_position = ciPosition + vec4(instance_position, 0);

	light_position = uLightPosition;

	light_intensity = uLightIntensity;
	normal_intensity = uNormalIntensity;

	diffuse_color = uDiffuseColor * instance_color;
	specular_color = uSpecularColor;
	emissive_color = uEmissiveColor;
	overlay_color = uOverlayColor;

	ambience = clamp(uAmbience, 0.0, 1.0);
	specularity = uSpecularity;
	luminosity = uLuminosity;
	roughness = clamp(uRoughness, 0.0001, 1.0);
	transparency = clamp(uTransparency, 0.0, 1.0);
	alpha = 1.0 - transparency;

	position = (ciModelView * vertex_position).xyz;
	normal = normalize(ciNormalMatrix * ciNormal);
	color = ciColor;
	texinates = transform_texinates(ciTexCoord0, uTextureScale, uTextureShift, uTextureRotation);
	overlay_texinates = transform_texinates(ciTexCoord0, uOverlayScale, uOverlayShift, uOverlayRotation);

	#ifdef HEIGHT_MAP
		float height = texture(uHeightMap, ciTexCoord0).r;
		vertex_position.xyz += ciNormal * height * uHeightScale;
	#endif

	gl_Position = ciModelViewProjection * vertex_position;
}
