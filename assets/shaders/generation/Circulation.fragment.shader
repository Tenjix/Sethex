// shadertype=glsl
#version 430

#include <shaders/Mathematics.include>

#ifdef ORIGIN_UPPER_LEFT
	layout(origin_upper_left) in vec4 gl_FragCoord;
#endif

uniform sampler2D uElevationMap;
uniform sampler2D uTemperatureMap;
uniform sampler2D uHumidityMap;
uniform sampler2D uFlowMap;

uniform float uIntensity = 1.0;

in vec2 Texinates;

out vec4 Output;

void main() {
	ivec2 resolution = textureSize(uElevationMap, 0);
	ivec2 texel = ivec2(gl_FragCoord);

	float elevation = texelFetch(uElevationMap, texel, 0).x;
	float temperature = texelFetch(uTemperatureMap, texel, 0).x;
	float humidity = texelFetch(uHumidityMap, texel, 0).x;
	vec2 flow = texelFetch(uFlowMap, texel, 0).xy;

	// circulate humidity

	vec2 flow_direction = normalize(flow);
	float flow_speed = length(flow);

	ivec2 circulated_texel = ivec2(gl_FragCoord.xy - flow_direction);
	float circulated_humidity = texelFetch(uHumidityMap, circulated_texel, 0).x;

	humidity += circulated_humidity * uIntensity;
	// humidity = mix(humidity, circulated_humidity, 0.5);

	Output.rgb = vec3(humidity);
	Output.a = 1.0;
	
}
