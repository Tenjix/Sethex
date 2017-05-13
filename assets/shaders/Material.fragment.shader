// shadertype=glsl
#version 150

// #define UNLIT
// #define DIFFUSE_TEXTURE
// #define SPECULAR_TEXTURE
// #define EMISSIVE_TEXTURE
// #define NORMAL_MAP
// #define OVERLAY_TEXTURE

#include <shaders/Normals.include>

uniform sampler2D uDiffuseTexture;
uniform sampler2D uSpecularTexture;
uniform sampler2D uEmissiveTexture;
uniform sampler2D uOverlayTexture;
uniform sampler2D uNormalMap;

flat in vec3 DiffuseColor;
flat in vec3 SpecularColor;
flat in vec3 EmissiveColor;
flat in vec3 OverlayColor;

flat in vec3 LightPosition;
flat in vec3 LightDirection;

flat in float LightIntensity;
flat in float NormalIntensity;

flat in float Ambience;
flat in float Specularity;
flat in float Luminosity;
flat in float Roughness;
flat in float Transparency;
flat in float Alpha;

in vec3 Position;
in vec3 Normal;
in vec3 Color;
in vec2 Texinates;
in vec2 OverlayTexinates;

out vec4 Output;

void main() {

	vec3 diffuse_color = DiffuseColor;
	vec3 specular_color = SpecularColor;
	vec3 emissive_color = EmissiveColor;
	vec3 overlay_color = OverlayColor;

	float light_intensity = LightIntensity;
	float normal_intensity = NormalIntensity;

	float ambience = Ambience;
	float specularity = Specularity;
	float luminosity = Luminosity;
	float roughness = Roughness;
	float transparency = Transparency;
	float alpha = Alpha;

	vec3 position = Position;
	vec3 color = Color;
	vec2 texinates = Texinates;
	vec2 overlay_texinates = OverlayTexinates;

	vec3 light_position = LightPosition;
	vec3 light_direction = normalize(Position - LightPosition);
	vec3 camera_direction = normalize(Position);
	vec3 normal_direction = normalize(Normal);

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

	#ifdef NORMAL_MAP
		vec3 mapped_normal = texture(uNormalMap, texinates).rgb;
		normal_direction = calculate_normal(normal_direction, -camera_direction, mapped_normal, texinates, normal_intensity);
	#endif

	#ifdef UNLIT
		Output.rgb = diffuse_color;
		Output.a = alpha;
	#else
		vec3 reflection_direction = normalize(reflect(light_direction, normal_direction));

		float diffuse_intensity = light_intensity * max(dot(normal_direction, -light_direction), 0.0);
		float specular_intensity = light_intensity * pow(max(dot(reflection_direction, -camera_direction), 0.0), 1.0 / roughness);
		float emissive_intensity = 1.0 + max(dot(normal_direction, -camera_direction), 0.0);

		vec3 diffuse = diffuse_color * max(diffuse_intensity, ambience);
		vec3 specular = specular_color * specular_intensity * specularity;
		vec3 emissive = emissive_color * emissive_intensity * luminosity;

		Output.rgb = diffuse + specular + emissive;
		Output.a = alpha;
	#endif

	#ifdef OVERLAY_TEXTURE
		vec4 overlay_color = texture(uOverlayTexture, overlay_texinates);
		Output.rgb = mix(Output.rgb, overlay_color.rgb * uOverlayColor, overlay_color.a);
		Output.a = max(Output.a, overlay_color.a);
	#endif

}
