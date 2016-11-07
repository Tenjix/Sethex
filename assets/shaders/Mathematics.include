// --------------------------------------------------------------------------------------------------------------------------
// provides matematical constants and functions
// --------------------------------------------------------------------------------------------------------------------------

const float Tau = 6.28318530717958647693;
const float Tau_Inverse = 1.0 / Tau;
const float Tau_Half = Tau / 2.0;
const float Pi = Tau_Half;
const float Pi_Inverse = 1.0 / Pi;
const float Pi_Half = Pi / 2.0;
const float Sqrt_2 = 1.41421356237309504880;
const float Sqrt_2_Inverse = 1.0 / Sqrt_2;
const float Sqrt_3 = 1.73205080756887729353;
const float Sqrt_3_Inverse = 1.0 / Sqrt_3;
const float One_Third = 1.0 / 3.0;
const float Two_Thirds = 2.0 / 3.0;

// returns 1.0 if "one" == "two", 0.0 otherwise 
float equal(float one, float two) {
	return step(one, two) * step(two, one);
}

// returns 1.0 if "value" < "other", 0.0 otherwise 
float smaller(float value, float other) {
	return 1.0 - step(other, value);
}

// returns 1.0 if "value" <= "other", 0.0 otherwise 
float smaller_or_equal(float value, float other) {
	return max(smaller(value, other), equal(value, other));
}

// returns 1.0 if "value" > "other", 0.0 otherwise 
float greater(float value, float other) {
	return step(other, value) * (1.0 - equal(value, other));
}

// returns 1.0 if "value" >= "other", 0.0 otherwise 
float greater_or_equal(float value, float other) {
	return step(other, value);
}

// returns 1.0 if "value" lies within [minimum, maximum], 0.0 otherwise 
float within(float value, float minimum, float maximum) {
	return greater_or_equal(value, minimum) * smaller_or_equal(value, maximum);
}

// returns 1.0 if "value" lies without of [minimum, maximum], 0.0 otherwise 
float without(float value, float minimum, float maximum) {
	return max(smaller(value, minimum), greater(value, maximum));
}

// creates a rotation matrix
mat2 rotation(float angle) {
	float sinus = sin(angle), cosinus = cos(angle);
	return mat2(cosinus, -sinus, sinus, cosinus);
}

// projects "value" into the wrapping range "range_begin" to "range_end"
int project(int value, int range_begin, int range_end) {
	int range_size = (range_end + 1) - range_begin;
	if (value < range_begin) value += range_size * ((range_begin - value) / range_size + 1);
	return range_begin + (value - range_begin) % range_size;
}
