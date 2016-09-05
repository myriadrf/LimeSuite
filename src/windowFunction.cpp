#include "windowFunction.h"
#include <cmath>

void GenerateWindowCoefficients(int func, int coefCount, std::vector<float> &windowFcoefs, float &amplitudeCorrection)
{
    windowFcoefs.clear();

    windowFcoefs.resize(coefCount);
    float a0 = 0.35875;
    float a1 = 0.48829;
    float a2 = 0.14128;
    float a3 = 0.01168;
    int N = coefCount;
	float PI = 3.14159265359;
    switch(func)
    {
	case 1: //blackman-harris
		for (int i = 0; i<N; ++i)
		{
			windowFcoefs[i] = a0 - a1*cos((2 * PI*i) / (N - 1)) + a2*cos((4 * PI*i) / (N - 1)) - a3*cos((6 * PI*i) / (N - 1));
			amplitudeCorrection += windowFcoefs[i];
		}
		amplitudeCorrection = 1.0 / (amplitudeCorrection / N);
		break;
    case 2: //hamming
        amplitudeCorrection = 0;
        a0 = 0.54;
        for(int i=0; i<N; ++i)
        {
            windowFcoefs[i] = a0 -(1-a0)*cos((2*PI*i)/(N));
            amplitudeCorrection += windowFcoefs[i];
        }
        amplitudeCorrection = 1.0/(amplitudeCorrection/N);
        break;
    case 3: //hanning
        amplitudeCorrection = 0;
        for(int i=0; i<N; ++i)
        {
            windowFcoefs[i] = 0.5 *(1 - cos((2*PI*i)/(N)));
            amplitudeCorrection += windowFcoefs[i];
        }
        amplitudeCorrection = 1.0/(amplitudeCorrection/N);
        break;
    default:
        for (int i = 0; i<N; ++i)
            windowFcoefs[i] = 1;
        amplitudeCorrection = 1;
    }
}
