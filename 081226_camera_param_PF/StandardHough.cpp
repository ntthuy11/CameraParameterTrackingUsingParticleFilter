#include "StdAfx.h"
#include "StandardHough.h"
#include "cv.h"
#include "highgui.h"

StandardHough::StandardHough(void) { }
StandardHough::~StandardHough(void) { }

void StandardHough::run(IplImage* img, CvSeq *linesFound, double rhoResolution, double thetaResolution, int threshold, int linesMax) {
	int height = img->height, width = img->width, step = img->widthStep;
	const uchar* imgData = (uchar *)img->imageData;

	int numOfTheta = generateSinThetaAndCosThetaList(thetaResolution);
	int numOfRho = cvRound( ((width + height)*2 + 1) / rhoResolution );
	int halfOfNumOfRho = cvRound(numOfRho / 2);

	// init satisfyList(accValue, thetaIndex, rhoIndex, 0 /* not used */ )
	CvMemStorage* satisfyListStorage = cvCreateMemStorage(0);
	CvSeq* satisfyList = cvCreateSeq(CV_SEQ_ELTYPE_GENERIC, sizeof(CvSeq), sizeof(CvRect), satisfyListStorage);

	// init accumulator
	CvMat* accumulator = cvCreateMat(numOfTheta, numOfRho, CV_32SC1);
	cvZero(accumulator);
	accCols = accumulator->cols;
	accData = accumulator->data.i;
	
	// select pixels and accumulate to accumulator
	for(int i = 0; i < height; i++) 
		for(int j = 0; j < width; j++) 
			if (imgData[i*step + j] > 0) accumulate(j, i, rhoResolution, numOfTheta, halfOfNumOfRho);

	// lay ra cac gia tri nao tich luy trong accumulator ma > threshold (co tim theo local maximum) de tao thanh duong thang
	for(int i = 0; i < numOfTheta; i++) 
		for(int j = 0; j < numOfRho; j++) {
			if (checkLocalMaximumWithWindow(5, 5, numOfTheta, numOfRho, i, j, threshold)) {
				int accValue = accData[i*accCols + j];
				CvRect s = { accValue, i, j, 0 }; // 0: not used
				cvSeqPush(satisfyList, &s);
			}
		}

	// sap xep satisfyList theo thu tu giam dan cua accValue
	cvSeqSort( satisfyList, cmp_func, 0 /* userdata is not used here */ );

	// lay ra n = linesMax phan tu de dua va linesFound
	for (int i = 0; i < min(linesMax, satisfyList->total); i++) {
		CvRect* s = (CvRect*)cvGetSeqElem(satisfyList, i);
		int thetaIndex = s->y, rhoIndex = s->width;

		double cosTheta = cosThetaList[thetaIndex], sinTheta = sinThetaList[thetaIndex];
		double rho = (rhoIndex - halfOfNumOfRho) * rhoResolution;
		double x0 = cosTheta*rho, y0 = sinTheta*rho;
		CvRect lr = { cvRound(x0 + 1000*(-sinTheta)), cvRound(y0 + 1000*(cosTheta)), 
					  cvRound(x0 - 1000*(-sinTheta)), cvRound(y0 - 1000*(cosTheta)) }; 
		cvSeqPush(linesFound, &lr);
	}

	// release
	cvReleaseMemStorage(&satisfyListStorage);
	cvReleaseMat(&accumulator);	
	delete sinThetaList;
	delete cosThetaList;
}

// ================= private =================

int StandardHough::generateSinThetaAndCosThetaList(double thetaResolution) { 
	// 180 degree [ -90 ; +90 ] ~ CV_PI radian [ -PI/2 ; PI/2 ]
	int numOfTheta = cvRound(CV_PI / thetaResolution) + 1; // cvRound: convert floating-point number to integer
	sinThetaList = new double[numOfTheta];
	cosThetaList = new double[numOfTheta];
	for (int i = 0; i < numOfTheta; i++) {
		double theta = i * thetaResolution - HALF_PI;
		sinThetaList[i] = roundDouble(sin(theta), DOUBLE_PRECISION);
		cosThetaList[i] = roundDouble(cos(theta), DOUBLE_PRECISION);
	}
	return numOfTheta;
}

void StandardHough::accumulate(int x, int y, double rhoResolution, int numOfTheta, int halfOfNumOfRho) {
	for (int i = 0; i < numOfTheta; i++) {
		double rho = x*cosThetaList[i] + y*sinThetaList[i];
		int indexRho = cvRound( rho / rhoResolution ) + halfOfNumOfRho;
		double remainder = rho - (indexRho - halfOfNumOfRho) * rhoResolution;
		if (remainder > rhoResolution/2) indexRho++;
		accData[i*accCols + indexRho] += 1;
	}
}

bool StandardHough::checkLocalMaximumWithWindow(int winHeight, int winWidth, int numOfTheta, int numOfRho, 
												   int rowOfCenter, int columnOfCenter, int threshold) { // 8-connectivity
	int accValue = accData[rowOfCenter*accCols + columnOfCenter];
	if (accValue <= threshold) return false;
	
	int startColumn = -min(winWidth, columnOfCenter);	int endColumn = min(winWidth, numOfRho - columnOfCenter - 1);
	int startRow = -min(winHeight, rowOfCenter);		int endRow = min(winHeight, numOfTheta - rowOfCenter - 1);

	for (int i = startRow; i <= endRow; i++) {
		for (int j = startColumn; j <= endColumn; j++) {
			if (i == 0 && j == 0) continue;

			int newRow = rowOfCenter + i;
			int newColumn = columnOfCenter + j;
			int acc = accData[newRow*accCols + newColumn];
			if ( (newRow < rowOfCenter) || (newRow == rowOfCenter && newColumn < columnOfCenter) ) {
				if (accValue <= acc) 
					return false;
			} else {
				if (accValue < acc) 
					return false;
			}
		}
	}

	return true;
}

// http://www.codeproject.com/cpp/floatutils.asp?df=100&forumid=208&exp=0&select=14154
double StandardHough::roundDouble(double doValue, int nPrecision) {
    static const double doBase = 10.0;
    double doComplete5, doComplete5i;
    
    doComplete5 = doValue * pow(doBase, (double) (nPrecision + 1));
    
    if(doValue < 0.0) doComplete5 -= 5.0;
    else doComplete5 += 5.0;
    
    doComplete5 /= doBase;
    modf(doComplete5, &doComplete5i);
    
    return doComplete5i / pow(doBase, (double) nPrecision);
}