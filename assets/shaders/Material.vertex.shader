// shadertype=glsl
#version 150

// #define INSTANTIATION
// #define HEIGHT_MAP

#include <shaders/Texinates.include>

uniform sampler2D uHeightMap;

uniform float uHeightScale = 1.0;

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelView;
uniform mat4 ciViewMatrix;
uniform mat3 ciNormalMatrix; 

uniform vec3 uLightPosition = vec3(0, 1000, 0);
uniform vec3 uLightDirection = vec3(0, 1, 0);

uniform float uLightIntensity = 1.0;
uniform float uNormalIntensity = 1.0;

uniform vec3 uDiffuseColor = vec3(1.0);
uniform vec3 uSpecularColor = vec3(1.0);
uniform vec3 uEmissiveColor = vec3(1.0);
uniform vec3 uOverlayColor = vec3(1.0);

uniform float uAmbience = 0.1;
uniform float uSpecularity = 0.1;
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

#ifdef INSTANTIATION
	in vec3 InstancePosition;
	in vec3 InstanceColor;
#endif

flat out vec3 DiffuseColor;
flat out vec3 SpecularColor;
flat out vec3 EmissiveColor;
flat out vec3 OverlayColor;

flat out vec3 LightPosition;

flat out float LightIntensity;
flat out float NormalIntensity;

flat out float Ambience;
flat out float Specularity;
flat out float Luminosity;
flat out float Roughness;
flat out float Transparency;
flat out float Alpha;

out vec3 Position;
out vec3 Normal;
out vec3 Color;
out vec2 Texinates;
out vec2 OverlayTexinates;

void main() {
	vec4 position = ciPosition;

	LightIntensity = clamp(uLightIntensity, 0.0, 1.0);
	NormalIntensity = uNormalIntensity;

	DiffuseColor = clamp(uDiffuseColor, 0.0, 1.0);
	SpecularColor = clamp(uSpecularColor, 0.0, 1.0);
	EmissiveColor = clamp(uEmissiveColor, 0.0, 1.0);
	OverlayColor = clamp(uOverlayColor, 0.0, 1.0);

	Ambience = clamp(uAmbience, 0.0, 1.0);
	Specularity = clamp(uSpecularity, 0.0, 1.0);
	Luminosity = clamp(uLuminosity, 0.0, 1.0);
	Roughness = clamp(uRoughness, 0.0001, 1.0);
	Transparency = clamp(uTransparency, 0.0, 1.0);
	Alpha = 1.0 - Transparency;

	#ifdef INSTANTIATION
		position += vec4(InstancePosition, 0);
		vec3 instance_color = clamp(InstanceColor, 0.0, 1.0);
		DiffuseColor = DiffuseColor * instance_color;
	#endif

	LightPosition = (ciViewMatrix * vec4(uLightPosition, 1)).xyz;
	Position = (ciModelView * position).xyz;
	Normal = normalize(ciNormalMatrix * ciNormal);
	Color = ciColor;
	Texinates = transform_texinates(ciTexCoord0, uTextureScale, uTextureShift, uTextureRotation);
	OverlayTexinates = transform_texinates(ciTexCoord0, uOverlayScale, uOverlayShift, uOverlayRotation);

	#ifdef HEIGHT_MAP
		float height = texture(uHeightMap, ciTexCoord0).r;
		position.xyz += ciNormal * height * uHeightScale;
	#endif

	gl_Position = ciModelViewProjection * position;
}
