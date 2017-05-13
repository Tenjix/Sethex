// shadertype=glsl
#version 330

#include <shaders/Mathematics.include>

#ifdef ORIGIN_UPPER_LEFT
	layout(origin_upper_left) in vec4 gl_FragCoord;
#endif

uniform sampler2D uTemperatureMap;

uniform float uEquator = 0.0;
uniform float uBaseDensityInfluence = 2.0;
uniform bool uDensityIncreasesWithTemperature = true;
uniform bool uDebug = false;

in vec2 Texinates;

out vec4 Output;

ivec2 map_size;

// calcualtes normalized distance to equator
float calculate_distance_to_equator(ivec2 texel) {
	float latitude = to_signed_range(float(texel.y) / map_size.y);
	float delta = latitude - uEquator;
	float range = 1.0 - sign(delta) * uEquator;
	return abs(delta / range);
}

// limits and projects "texel" onto a horizontally wrapping map with "map_size"
ivec2 limit(ivec2 texel, ivec2 map_size) {
	texel.x = project(texel.x, 0, map_size.x - 1);
	texel.y = clamp(texel.y, 0, map_size.y - 1);
	return texel;
}

// reads the temperature of the given texel from the texture
float get_temperature(ivec2 texel) {
	return texelFetch(uTemperatureMap, texel, 0).x;
}

// calculates air density based on circulation cells
float calculate_base_density(ivec2 texel) {
	float verticality = to_unsigned_range(calculate_distance_to_equator(texel));
	return to_unsigned_range(cos(verticality * 3 * Tau));
}

// calculates air density based on circulation cells and temperature
float calculate_density(ivec2 texel) {
	texel = limit(texel, map_size);
	float base = calculate_base_density(texel);
	float deviation = uDensityIncreasesWithTemperature? get_temperature(texel) : 1.0 - get_temperature(texel);
	return (uBaseDensityInfluence * base + deviation) / (uBaseDensityInfluence + 1.0);
}

// calculates air density deltas between opposing neighbor cells of the given texel
vec4 calculate_density_delta(ivec2 texel, int distance = 1) {
	vec4 delta;
	// west - east
	delta.x = calculate_density(texel + ivec2(-distance, 0)) - calculate_density(texel + ivec2(distance, 0));
	// south - south
	delta.y = calculate_density(texel + ivec2(0, -distance)) - calculate_density(texel + ivec2(0, distance));
	// south-west - north-east
	delta.z = calculate_density(texel + ivec2(-distance, -distance)) - calculate_density(texel + ivec2(distance, distance));
	// north-west - south-east
	delta.w = calculate_density(texel + ivec2(-distance, distance)) - calculate_density(texel + ivec2(distance, -distance));
	return delta;
}

// calculates air exchange based on air dentity deltas surrounding the given texel
vec4 calculate_air_exchange(ivec2 texel, int octaves = 1, float decline = 0.5) {
	vec4 exchange = vec4(0.0);
	float intensity = 1.0;
	float range = 0.0;
	for (int octave = 0; octave < octaves; octave++) {
		exchange += intensity * calculate_density_delta(texel, int(exp2(octave)));
		range += intensity;
		intensity *= decline;
	}
	return exchange / range;
}

// deflects air flow depending on hemisphere and flow spead
vec2 apply_coriolis_effect(ivec2 texel, vec2 flow) {
	float latitude = float(texel.y) / map_size.y;
	float equator = to_unsigned_range(uEquator);
	float direction = sign(latitude - equator);
	return rotation(Tau_Quarter * direction * length(flow)) * flow;
}

// calculates air flow at the given texel based on air exchange
vec2 calculate_air_flow(ivec2 texel) {
	vec2 east_direction = vec2(1, 0);
	vec2 north_direction = vec2(0, 1);
	vec2 northeast_direction = vec2(Sqrt_2_Inverse);
	vec2 southeast_direction = vec2(Sqrt_2_Inverse, -Sqrt_2_Inverse);

	vec4 exchange = calculate_air_exchange(texel, 7);
	float exchange_coefficient = 1.5;
	exchange = clamp(exchange * exchange_coefficient, -1.0, 1.0);

	vec2 flow = east_direction * exchange.x + north_direction * exchange.y + northeast_direction * exchange.z + southeast_direction * exchange.w;
	flow = apply_coriolis_effect(texel, flow);

	return flow;
}

// calculates the air flow gradient of the given pixel
vec3 calculate_gradient(ivec2 texel) {
	// return normalize(vec3(convert_flow_to_color(calculate_flow(texel)), 1.0));
	vec2 flow = calculate_air_flow(texel);
	return normalize(vec3(flow.x, 1.0, flow.y));
}

void main() {

	map_size = textureSize(uTemperatureMap, 0);
	ivec2 texel = ivec2(gl_FragCoord);
	
	vec2 flow = calculate_air_flow(texel);

	Output.rg = normalize(flow);
	Output.b = length(flow);
	Output.a = 1.0;

	if(uDebug) {
		Output.rgb = vec3(0.0);
		if(Texinates.x < 0.25) {
			Output.b = calculate_density(texel);
		} else if(Texinates.x < 0.5) {
			Output.b = length(flow)*2.5;
		} else if(Texinates.x < 0.75) {
			Output.r = max(-flow.y*1.5, 0.0);
			Output.g = max(flow.y*1.5, 0.0);
		} else {
			Output.r = max(-flow.x*3.0, 0.0);
			Output.g = max(flow.x*3.0, 0.0);
		}
	}

}
