// --------------------------------------------------------------------------------------------------------------------------
// provides functions to convert between alpha and transparency values
// --------------------------------------------------------------------------------------------------------------------------

float alpha_to_transparency(float alpha) {
	return 1.0 - pow(clamp(alpha, 0.0, 1.0), 2.0);
}

float transparency_to_alpha(float transparency) {
	return sqrt(1.0 - clamp(transparency, 0.0, 1.0));
}
