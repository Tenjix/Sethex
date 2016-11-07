// --------------------------------------------------------------------------------------------------------------------------
// provides functions to calculate the normal in tangent space efficiently using covectors
// based on http://www.thetenthplanet.de/archives/1180
// --------------------------------------------------------------------------------------------------------------------------

mat3 calculate_tangent_matrix(vec3 vertex_normal, vec3 camera_direction, vec2 texture_coordinates) {
	// get edge vectors of the pixel triangle
	vec3 camera_direction_dx = dFdx(camera_direction);
	vec3 camera_direction_dy = dFdy(camera_direction);
	vec2 texture_coordinates_dx = dFdx(texture_coordinates);
	vec2 texture_coordinates_dy = dFdy(texture_coordinates);

	// solve the linear system
	vec3 horizontal = cross(camera_direction_dy, vertex_normal);
	vec3 vertical = cross(vertex_normal, camera_direction_dx);
	vec3 tangent = horizontal * texture_coordinates_dx.x + vertical * texture_coordinates_dy.x;
	vec3 bitangent = horizontal * texture_coordinates_dx.y + vertical * texture_coordinates_dy.y;

	// construct a scale-invariant frame 
	float inverse_maximum = inversesqrt(max(dot(tangent, tangent), dot(bitangent, bitangent)));
	return mat3(tangent * inverse_maximum, bitangent * inverse_maximum, vertex_normal);
}

vec3 calculate_normal(vec3 vertex_normal, vec3 direction_to_camera, vec3 mapped_normal, vec2 texture_coordinates, float intensity = 1.0) {

	#ifndef NORMALMAP_SIGNED
		mapped_normal = mapped_normal * 255.0/127.0 - 128.0/127.0;
	#endif

	#ifdef NORMALMAP_2CHANNEL
		mapped_normal.z = sqrt(1.0 - dot(mapped_normal.xy, mapped_normal.xy));
	#endif

	#ifdef NORMALMAP_GREEN_UP
		mapped_normal.y = -mapped_normal.y;
	#endif

	#ifdef NORMALMAP_INVERT
		mapped_normal.z = -mapped_normal.z;
	#endif

	#ifndef NORMALMAP_IGNORE_INTENSITY
		mapped_normal.y *= sign(intensity);
		mapped_normal.z /= clamp(abs(intensity), 0.01, 100.0);
	#endif

	mat3 tangent_matrix = calculate_tangent_matrix(vertex_normal, -direction_to_camera, texture_coordinates);
	return normalize(tangent_matrix * mapped_normal);
}

vec3 calculate_normal(vec3 vertex_normal, vec3 direction_to_camera, sampler2D normal_map, vec2 texture_coordinates, float intensity = 1.0) {
	return calculate_normal(vertex_normal, direction_to_camera, texture2D(normal_map, texture_coordinates).rgb, texture_coordinates, intensity);
}
