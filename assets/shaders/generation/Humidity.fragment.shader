// shadertype=glsl
#version 430

#include <shaders/Mathematics.include>

#ifdef ORIGIN_UPPER_LEFT
	layout(origin_upper_left) in vec4 gl_FragCoord;
#endif

uniform sampler2D uElevationMap;
uniform sampler2D uTemperatureMap;
uniform sampler2D uCirculationMap;
uniform sampler2D uHumidityMap;

uniform float uSeaLevel = 0.0;
uniform float uIntensity = 1.0;
uniform uint uIteration = 1;

in vec2 Texinates;

out vec4 Output;

void main() {

	ivec2 resolution = textureSize(uElevationMap, 0);
	ivec2 texel = ivec2(gl_FragCoord);

	float sea_level = to_unsigned_range(uSeaLevel);
	float elevation = texture(uElevationMap, Texinates).r;
	float temperature = texture(uTemperatureMap, Texinates).r;
	float humidity = texture(uHumidityMap, Texinates).r;
	vec2 wind_direction = normalize(texture(uCirculationMap, Texinates).rg);
	float wind_speed = texture(uCirculationMap, Texinates).b;

	bool land = elevation > sea_level;
	float intensity = float(land) * uIntensity * 0.5;
	float scale = float(uIteration) * 0.05;
	// scale = 0.1;

	// circulate humidity

	float inflow_humidity = texture(uHumidityMap, Texinates - wind_direction * wind_speed * scale).x;
	float outflow_humidity = texture(uHumidityMap, Texinates + wind_direction * wind_speed * scale).x;

	float inflow = max(inflow_humidity - humidity, 0.0);
	float outflow = max(humidity - outflow_humidity, 0.0) * 0.5;
	humidity += inflow * intensity;
	humidity -= outflow * intensity;

	Output.r = humidity;
	Output.a = 1.0;
	
}
