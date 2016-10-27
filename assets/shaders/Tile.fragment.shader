// shadertype=glsl
#version 150

#include <shaders/Mathematics.h>
#include <shaders/Normals.h>

uniform sampler2D uDiffuseTexture;
uniform sampler2D uSpecularTexture;
uniform sampler2D uEmissiveTexture;
uniform sampler2D uOverlayTexture;
uniform sampler2D uNormalMap;

flat in vec3 diffuse_color;
flat in vec3 specular_color;
flat in vec3 emissive_color;
flat in vec3 overlay_color;
flat in vec3 light_position;

flat in float light_intensity;
flat in float normal_intensity;

flat in float ambience;
flat in float specularity;
flat in float luminosity;
flat in float roughness;
flat in float transparency;
flat in float alpha;

in vec3 position;
in vec3 normal;
in vec3 color;
in vec2 texinates;
in vec2 overlay_texinates;

out vec4 final;

void main() {

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

	vec3 direction_to_light = normalize(light_position - position);
	vec3 direction_to_camera = normalize(-position);
	vec3 normal_direction = normal;

	#ifdef NORMAL_MAP
		vec3 mapped_normal = texture(uNormalMap, texinates).rgb;
		normal_direction = calculate_normal(normal_direction, direction_to_camera, mapped_normal, texinates, normal_intensity);
	#endif

	#ifdef UNLIT
		final.rgb = diffuse_color;
		final.a = alpha;
	#else
		vec3 reflection_direction = normalize(reflect(-direction_to_light, normal_direction));

		float diffuse_intensity = light_intensity * max(dot(normal_direction, direction_to_light), 0.0);
		float specular_intensity = light_intensity * pow(max(dot(reflection_direction, direction_to_camera), 0.0), 1.0 / roughness);
		float emissive_intensity = 1.0 + max(dot(normal_direction, direction_to_camera), 0.0);

		vec3 diffuse = diffuse_color * max(diffuse_intensity, ambience);
		vec3 specular = specular_color * specular_intensity * specularity;
		vec3 emissive = emissive_color * emissive_intensity * luminosity;

		final.rgb = diffuse + specular + emissive;
		final.a = alpha;
	#endif

	#ifdef OVERLAY_TEXTURE
		vec4 overlay_color = texture(uOverlayTexture, overlay_texinates);
		final.rgb = mix(final.rgb, overlay_color.rgb * uOverlayColor, overlay_color.a);
		final.a = max(final.a, overlay_color.a);
	#endif

}
