#ifndef PERLIN_NOISE_INC
#define PERLIN_NOISE_INC
#include <cmath>

namespace EnjoLib
{
/// Provided by Oliver Pieper "Redburne" from CamShake project
class PerlinNoise
{
public:
    double IntNoise(int x) const;
    double SmoothNoise1(int x) const;
    double Interpolate(double a, double b, double x) const;
    double InterpolatedNoise(double x) const;
    double Perlin(double x, double freqamp[], int count) const;
};
}

#endif // PERLIN_NOISE_INC
