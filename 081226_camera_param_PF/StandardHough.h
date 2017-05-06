#pragma once
#include "cv.h"
#include "highgui.h"

#define DOUBLE_PRECISION (2)
#define HALF_PI (1.5707963) 

class StandardHough
{
public:
	StandardHough(void);
public:
	~StandardHough(void);

	// main method
	void run(IplImage* src, CvSeq *linesFound, double rhoResolution, double thetaResolution, int threshold, int linesMax);

private:
	double* sinThetaList;	
	double* cosThetaList;

	int *accData;
	int accCols;
	
	int generateSinThetaAndCosThetaList(double thetaResolution);
	void accumulate(int x, int y, double rhoResolution, int numOfTheta, int halfOfNumOfRho);
	bool checkLocalMaximumWithWindow(int winHeight, int winWidth, int numOfTheta, int numOfRho, int rowOfCenter, int columnOfCenter, int threshold);
	double roundDouble(double doValue, int nPrecision);

	static int cmp_func( const void* _a, const void* _b, void* userdata ) {
		CvRect* a = (CvRect*)_a;
		CvRect* b = (CvRect*)_b;
		int accValueA = a->x;
		int accValueB = b->x;
		return accValueB - accValueA;
	}
};
