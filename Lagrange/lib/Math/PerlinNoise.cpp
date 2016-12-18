#include "PerlinNoise.hpp"
#include <cmath>

using namespace EnjoLib;

double PerlinNoise::IntNoise(int x) const {
    // Generate a fixed pseudo-random float value for a given integer x.
    x = (x << 13) ^ x;
    return (1.0 - ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

double PerlinNoise::SmoothNoise1(int x) const {
    return IntNoise(x)/2.0 + IntNoise(x-1)/4.0 + IntNoise(x+1)/4.0;
}

double PerlinNoise::Interpolate(double a, double b, double x) const {
    // Return the cosine interpolation from a to b for 0.0 <= x <= 1.0
    double ft = x * 3.1415927;
    double f = (1 - cos(ft)) * 0.5;
    return  a*(1-f) + b*f;
}

double PerlinNoise::InterpolatedNoise(double x) const {
	int integer_X    = int(x);
	double fractional_X = x - integer_X;

	double v1 = SmoothNoise1(integer_X);
	double v2 = SmoothNoise1(integer_X + 1);

	return Interpolate(v1 , v2 , fractional_X);
}

double PerlinNoise::Perlin(double x, double freqamp[], int count) const {
	double total = 0.0;

	for (int i=0; i<count; i++) {
		total = total + InterpolatedNoise(x * freqamp[2*i]) * freqamp[2*i + 1];
	}
	return total;
}
