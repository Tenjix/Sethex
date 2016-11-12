// shadertype=glsl
#version 430

#include <shaders/Mathematics.include>

#ifdef ORIGIN_UPPER_LEFT
	layout(origin_upper_left) in vec4 gl_FragCoord;
#endif

uniform sampler2D uElevationMap;
uniform sampler2D uTemperatureMap;

in vec2 Texinates;

out vec4 Output;

ivec2 map_size;

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
	float verticality = float(texel.y) / map_size.y;
	return (cos(verticality * 6 * Tau_Half) + 1.0) / 2.0;
}

// calculates air density based on circulation cells and temperature
float calculate_density(ivec2 texel) {
	texel = limit(texel, map_size);
	// return calculate_base_density(texel);
	return (calculate_base_density(texel) + get_temperature(texel)) / 2.0;
}

// calculates air density deltas between opposing neighbor cells of the given texel
vec4 calculate_density_delta(ivec2 texel, int distance = 1) {
	vec4 delta;
	// north - south
	delta.x = calculate_density(texel - ivec2(0, -distance)) - calculate_density(texel - ivec2(0, distance));
	// west - east
	delta.y = calculate_density(texel - ivec2(-distance, 0)) - calculate_density(texel - ivec2(distance, 0));
	// north-west - south-east
	delta.z = calculate_density(texel - ivec2(-distance, -distance)) - calculate_density(texel - ivec2(distance, distance));
	// south-west - north-east
	delta.w = calculate_density(texel - ivec2(-distance, distance)) - calculate_density(texel - ivec2(distance, -distance));
	return delta;
}

// calculates air pressure based on air dentity deltas surrounding the given texel
vec4 calculate_pressure(ivec2 texel, int octaves = 1, float decline = 0.5) {
	vec4 pressure = vec4(0.0);
	float intensity = 1.0;
	float range = 0.0;
	for (int octave = 0; octave < octaves; octave++) {
		pressure += intensity * calculate_density_delta(texel, int(exp2(octave)));
		range += intensity;
		intensity *= decline;
	}
	return pressure / range;
}

// deflects air flow depending on hemisphere and flow spead
vec2 apply_coriolis_effect(vec2 texel, vec2 flow) {
	float verticality = float(texel.y) / map_size.y;
	float direction = -sign(0.5 - verticality);
	// todo: maybe don't deflect horizontal flow?
	return rotation(Pi_Half * direction * length(flow)) * flow;
}

// calculates air flow at the given texel based on air pressure
vec2 calculate_flow(ivec2 texel) {
	vec2 north_south_direction = vec2(0, 1);
	vec2 west_east_direction = vec2(1, 0);
	vec2 northwest_southeast_direction = vec2(Sqrt_2_Inverse);
	vec2 southwest_northeast_direction = vec2(Sqrt_2_Inverse, -Sqrt_2_Inverse);

	vec4 pressure = calculate_pressure(texel, 7);

	// float pressure_coefficient = 1.5;
	// pressure = clamp(pressure * pressure_coefficient, -1.0, 1.0);

	vec2 flow = north_south_direction * pressure.x 
		+ west_east_direction * pressure.y 
		+ northwest_southeast_direction * pressure.z 
		+ southwest_northeast_direction * pressure.w;

	flow = apply_coriolis_effect(texel, flow);
	// debugUnsignedUnitRangeDual(flow.x);

	return flow;
}

// calculates the air flow gradient of the given pixel
vec3 calculate_gradient(ivec2 texel) {
	// return normalize(vec3(convert_flow_to_color(calculate_flow(texel)), 1.0));
	vec2 flow = calculate_flow(texel);
	return normalize(vec3(flow.x, 1.0, flow.y));
}

void main() {

	map_size = textureSize(uTemperatureMap, 0);
	ivec2 texel = ivec2(gl_FragCoord);
	
	vec2 flow = calculate_flow(texel);
	flow = to_unsigned_range(flow);

	Output.rg = flow;
	Output.b = 0.0;
	Output.a = 1.0;

}
