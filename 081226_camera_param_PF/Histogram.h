#pragma once

#include "cv.h"
#include "highgui.h"

#define HIST_BINS	(256)
#define GRAY_LEVEL	(255)

class Histogram
{
public:
	Histogram(void);
	~Histogram(void);

	static double calculateEntropy(IplImage* srcImg) {  // srcImg is a IPL_DEPTH_8S image
		int numBins = HIST_BINS*2 - 1; // [-255 -> 255]
		int *grayHist = new int[numBins];
		for (int i = 0; i < numBins; i++) grayHist[i] = 0;

		Histogram::calculateGrayHist(srcImg, grayHist);
		double entropy = calculateEntropy(grayHist, numBins);

		delete[] grayHist;
		return entropy;
	}

	static void calculateGrayHist(IplImage* img, int* grayHist) { // img phai la anh xa'm, grayHist co kich thuoc la 256*2 - 1 (tu`y)
		int height = img->height, width = img->width;
		const uchar* imgData = (uchar *)img->imageData;
		for (int i = 0; i < height; i++) 
			for (int j = 0; j < width; j++) {
				int grayVal = imgData[i*width + j];
				grayHist[grayVal]++;
			}
	}

	static double calculateEntropy(int* hist, int size) { // Image Processing, Analysis, and Machine Vision (2007)
		// calculate sum of hist
		int sum = 0;
		for (int i = 0; i < size; i++) sum += hist[i];

		// calculate entropy
		double entropy = 0;
		for (int i = 0; i < size; i++)
			if (hist[i] != 0) {
				double prob = (hist[i] * 1.0 / sum);
				entropy += prob * Histogram::logBase(prob, 2);
			}

		return -entropy;
	}

	static double logBase(double n, double base) {
		return log(n) / log(base);
	}

	static void normalizeIntHist(int* hist, double* normHist, int size) {
		int sumOfHistogramValue = 0;
		for (int i = 0; i < size; i++) sumOfHistogramValue += hist[i];
		for (int i = 0; i < size; i++) normHist[i] = hist[i] * 1.0 / sumOfHistogramValue;
	}

	static void normalizeDoubleHist(double* hist, double* normHist, int size) {
		double sumOfHistogramValue = 0;
		for (int i = 0; i < size; i++) sumOfHistogramValue += hist[i];
		for (int i = 0; i < size; i++) normHist[i] = hist[i] / sumOfHistogramValue;
	}

	static void printIntHistToFile(LPCTSTR filename, int* hist, int size) {
		CStdioFile f;		f.Open(filename, CFile::modeCreate | CFile::modeWrite);
		CString text;
		for (int i = 0; i < size; i++) {
			text.Format(L"%7d\n", hist[i]);
			f.WriteString(text);
		}
		f.Close();
	}

	static void printDoubleHistToFile(LPCTSTR filename, double* hist, int size) {
		CStdioFile f;		f.Open(filename, CFile::modeCreate | CFile::modeWrite);
		CString text;
		for (int i = 0; i < size; i++) {
			text.Format(L"%3.5f\n", hist[i]);
			f.WriteString(text);
		}
		f.Close();
	}
};
