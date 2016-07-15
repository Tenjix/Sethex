// shadertype=glsl
#version 150

// #undef DIFFUSE_TEXTURE
// #undef SPECULAR_TEXTURE
// #undef EMISSIVE_TEXTURE
// #undef NORMAL_MAP

#define Pi_Half = 1.57079632679489661923;

#include <shaders/Normals.h>

uniform sampler2D uDiffuseTexture;
uniform sampler2D uSpecularTexture;
uniform sampler2D uEmissiveTexture;
uniform sampler2D uOverlayTexture;
uniform sampler2D uNormalMap;

uniform vec3 uLightPosition = vec3(0, 1, 0);
uniform float uLightIntensity = 1.0;

uniform vec3 uDiffuseColor = vec3(1.0);
uniform vec3 uSpecularColor = vec3(1.0);
uniform vec3 uEmissiveColor = vec3(1.0);
uniform vec3 uOverlayColor = vec3(1.0);
uniform float uNormalIntensity = 1.0;

uniform float uAmbience = 0.1;
uniform float uSpecularity = 0.0;
uniform float uLuminosity = 0.0;
uniform float uRoughness = 0.1;
uniform float uTransparency = 0.0;

uniform vec2 uTextureScale = vec2(1.0);
uniform vec2 uTextureShift = vec2(0.0);
uniform mat2x2 uTextureRotation = mat2x2(0.0, 1.0, -1.0, 0.0);

in vec4 vPosition;
in vec3 vNormal;
in vec2 vTexCoord0;

out vec4 oColor;

vec2 scale_texinates(vec2 texinates, vec2 scaling, bool centered = true) {
	scaling = max(scaling, 0.001);
	return centered? ((texinates - 0.5) / scaling + 0.5) : (texinates / scaling);
}

vec2 rotate_texinates(vec2 texinates, mat2x2 rotation, bool centered = true) {
	return centered? (rotation * (texinates - 0.5) + 0.5) : (rotation * texinates);
}

void main() {
	vec3 light_position = uLightPosition;
	float light_intensity = uLightIntensity;

	float ambience = clamp(uAmbience, 0.0, 1.0);
	float specularity = uSpecularity;
	float luminosity = uLuminosity;
	float roughness = clamp(uRoughness, 0.0001, 1.0);
	float transparency = clamp(uTransparency, 0.0, 1.0);
	float alpha = 1.0 - transparency;

	vec2 texinates = scale_texinates(vTexCoord0, uTextureScale) - uTextureShift;
	texinates = rotate_texinates(texinates, uTextureRotation);

	vec3 diffuse_color = uDiffuseColor;
	vec3 specular_color = uSpecularColor;
	vec3 emissive_color = uEmissiveColor;

	#ifdef DIFFUSE_TEXTURE
		vec4 mapped_diffuse = texture(uDiffuseTexture, texinates);
		diffuse_color *= mapped_diffuse.rgb;
		alpha *= mapped_diffuse.a;
	#endif

	#ifdef SPECULAR_TEXTURE
		specular_color *= texture(uSpecularTexture, texinates).rgb;
		roughness *= (specular_color.r + specular_color.g + specular_color.b) / 3;
	#endif

	#ifdef EMISSIVE_TEXTURE
		emissive_color *= texture(uEmissiveTexture, texinates).rgb;
	#endif

	vec3 position = vPosition.xyz;
	vec3 direction_to_light = normalize(light_position - position);
	vec3 direction_to_camera = normalize(-position);
	vec3 normal_direction = normalize(vNormal);

	#ifdef NORMAL_MAP
		vec3 mapped_normal = texture(uNormalMap, texinates).rgb;
		normal_direction = calculate_normal(normal_direction, direction_to_camera, mapped_normal, texinates, uNormalIntensity);
	#endif

	vec3 reflection_direction = normalize(reflect(-direction_to_light, normal_direction));

	float diffuse_intensity = light_intensity * max(dot(normal_direction, direction_to_light), 0.0);
	float specular_intensity = light_intensity * pow(max(dot(reflection_direction, direction_to_camera), 0.0), 1.0 / roughness);
	float emissive_intensity = 1.0 + max(dot(normal_direction, direction_to_camera), 0.0);

	vec3 diffuse = diffuse_color * max(diffuse_intensity, ambience);
	vec3 specular = specular_color * specular_intensity * specularity;
	vec3 emissive = emissive_color * emissive_intensity * luminosity;

	oColor.rgb = diffuse + specular + emissive;
	oColor.a = alpha;

	#ifdef OVERLAY_TEXTURE
		vec4 overlay_color = texture(uOverlayTexture, scale_texinates(texinates, vec2(0.5)));
		overlay_color.rgb *= uOverlayColor;
		oColor = mix(oColor, overlay_color, overlay_color.a);		
	#endif
}
