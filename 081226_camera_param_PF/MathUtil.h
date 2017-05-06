#pragma once

#include "cv.h"
#include "Histogram.h"

class MathUtil
{
public:
	MathUtil(void);
	~MathUtil(void);

	static double bhattacharyyaCoefficient(double* sampleDist, double* dist) {
		double result = 0;
		for (int i = 0; i < HIST_BINS; i++) 
			result += sqrt(sampleDist[i]*dist[i]);
		return result;
	}

	static double gaussianWeight(double bhaCoeff, double sigma) {
		double factor = 1.0/(sqrt(2*CV_PI)*sigma);
		return factor * exp( - (1-bhaCoeff) / (2*sigma*sigma) );
	}
};
