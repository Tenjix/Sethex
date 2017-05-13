// --------------------------------------------------------------------------------------------------------------------------
// provides functions to scale, shift and rotate texinates
// --------------------------------------------------------------------------------------------------------------------------

vec2 scale_texinates(vec2 texinates, vec2 scaling, bool centered = true) {
	scaling = max(scaling, 0.001);
	return centered? ((texinates - 0.5) / scaling + 0.5) : (texinates / scaling);
}

vec2 shift_texinates(vec2 texinates, vec2 shift) {
	return texinates - shift;
}

vec2 rotate_texinates(vec2 texinates, mat2x2 rotation, bool centered = true) {
	return centered? (rotation * (texinates - 0.5) + 0.5) : (rotation * texinates);
}

vec2 transform_texinates(vec2 texinates, vec2 scale, vec2 shift, mat2x2 rotation, bool centered = true) {
	return rotate_texinates(shift_texinates(scale_texinates(texinates, scale, centered), shift), rotation, centered);
}
