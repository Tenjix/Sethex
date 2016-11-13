// shadertype=glsl
#version 330

#include <shaders/Mathematics.include>
#include <shaders/Noise.include>

#ifdef ORIGIN_UPPER_LEFT
	layout(origin_upper_left) in vec4 gl_FragCoord;
#endif

uniform bool uWrapping = false;
uniform uvec2 uResolution;
uniform int uSeed;
uniform vec2 uShift = vec2(0.0, 0.0);
uniform float uScale = 1.0;
uniform uint uOctaces = 10u;
uniform float uAmplitude = 1.0;
uniform float uFrequency = 1.0;
uniform float uLacunarity = 2.0;
uniform float uPersistence = 0.5;
uniform float uPower = 1.0;

uniform float uContinentalAmplitudeFactor = 2.0;
uniform float uContinentalFrequencyFactor = 0.25;
uniform vec2 uContinentalShift = vec2(0.0, 0.0);

uniform float uEquatorDistanceFactor = 0.0;
uniform int uEquatorDistancePower = 1;

in vec2 Texinates;

out vec4 Output;

void main() {

	float distance_to_equator = abs(2.0 * (Texinates.y - 0.5));

	float continental_elevation;
	float regional_elevation;
	
	vec2 center = vec2(uResolution) / 2.0;
	vec2 position = gl_FragCoord.xy;
	if (uWrapping) {
		float repeat_interval = uResolution.x / uScale;
		position.y = (position.y - center.y) / uScale + center.y;
		position += uShift;
		float radians = position.x / uResolution.x * Tau;
		vec3 cylindrical_position;
		cylindrical_position.x = sin(radians) / Tau * repeat_interval;
		cylindrical_position.y = position.y;
		cylindrical_position.z = cos(radians) / Tau * repeat_interval;
		cylindrical_position *= 0.01;
		continental_elevation = simplex_noise(cylindrical_position * uFrequency * uContinentalFrequencyFactor);
		regional_elevation = simplex_noise(cylindrical_position * uFrequency, int(uOctaces), uLacunarity, uPersistence, uPower);
	} else {
		position = (position - center) / uScale + center;
		position += uShift;
		position *= 0.01;
		continental_elevation = simplex_noise(position * uFrequency * uContinentalFrequencyFactor);
		regional_elevation = simplex_noise(position * uFrequency, int(uOctaces), uLacunarity, uPersistence, uPower);
	}

	float elevation = (uContinentalAmplitudeFactor * continental_elevation + regional_elevation) / (uContinentalAmplitudeFactor + 1.0);
	elevation += uEquatorDistanceFactor * pow(distance_to_equator, uEquatorDistancePower);
	elevation = clamp(uAmplitude * elevation, -1.0, 1.0);
	elevation = to_unsigned_range(elevation);

	Output.r = elevation;
	Output.a = 1.0;
}
