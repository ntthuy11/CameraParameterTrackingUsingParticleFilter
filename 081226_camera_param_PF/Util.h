#pragma once

#include "cv.h"
#include "highgui.h"

class Util
{
public:
	Util(void);
	~Util(void);

	static void generateRandomNumbers(int numOfRandomNumbers, int max, int *result) { // result is from 0 to max
		for (int i = 0; i < numOfRandomNumbers; i++) {
			CvRNG rng_state = cvRNG(GetTickCount() + i);
			result[i] = cvRandInt(&rng_state) % max;
		}
	}

	static void sobelDerivative(IplImage* srcImg, IplImage* resultImg) {
		IplImage* tmpSobelImg = cvCreateImage(cvSize(srcImg->width, srcImg->height), IPL_DEPTH_16S, 1);
		cvSobel(srcImg, tmpSobelImg, 1, 1, 3);
		cvConvertScaleAbs(tmpSobelImg, resultImg);
		cvReleaseImage(&tmpSobelImg);
	}

	static double energyOfLowPassFilteredImageGradient(IplImage* img, int filterWinSize) { // Subbarao M. – Focusing Techniques
		
		// low-pass filter the image (or blur, or smooth) 
		IplImage* lowPassImg = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_8U, 1);
		cvSmooth(img, lowPassImg, CV_GAUSSIAN, filterWinSize);

		// calculate energy
		double energy = .0;
		int height = lowPassImg->height, width = lowPassImg->width;
		const uchar* lowPassImgData = (uchar *)lowPassImg->imageData;
		for (int i = 0; i < height - 1; i++) { // khong tinh tren bottom border
			for (int j = 0; j < width - 1; j++) { // khong tinh tren right border
				int currPos = i*width + j;
				int rightPos = currPos + 1;
				int bottomPos = (i + 1)*width + j;
				int gx = lowPassImgData[rightPos] - lowPassImgData[currPos];
				int gy = lowPassImgData[bottomPos] - lowPassImgData[currPos];
				energy += gx*gx + gy*gy;
			}
		}

		// release
		cvReleaseImage(&lowPassImg);

		return energy;
	}
};
