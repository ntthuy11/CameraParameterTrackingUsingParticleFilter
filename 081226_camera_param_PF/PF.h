#pragma once

#include "cv.h"
#include "highgui.h"
#include "UniformGen.h"
#include "Histogram.h"
#include "Camera.h"
#include "MathUtil.h"
#include "Util.h"

typedef struct LevelParticle {
	int positionExp;
	int positionFocus;
	int velocityExp;
	int velocityFocus;
	double weight;

	CGaussianGen gaussianGenPositionExp; // moi particle se co 1 bo^. phat sinh so ngau nhien rieng biet & ko thay doi bo^. phat sinh nay theo thoi gian
	CGaussianGen gaussianGenPositionFocus;
	CGaussianGen gaussianGenVelocityExp;
	CGaussianGen gaussianGenVelocityFocus;
} LevelParticle;

// ----------------------------

class PF
{
public:
	PF(void);
	~PF(void);

	LevelParticle* particles;
	int numParticles, initExpLevel, initFocusLevel;
	double positionSigma, velocitySigma;
	double* previousFrameHistogram;

	Camera camera;
	int levelAprMax;

	// --------------------------

	void setParamsAndInit(int numParticles, double positionSigma, double velocitySigma, IplImage* initFrame, int initExpLevel, int initFocusLevel);
	CvPoint process(CString imgSetPath, int frameWidth, int frameHeight, int expLevelMax, int focusLevelMax);	

private:
	void initParticles();
	void predict(int expLevelMax, int focusLevelMax);
	void calculateParticlesInfo(CString imgSetPath, int frameWidth, int frameHeight);
	void normalizeWeights(LevelParticle* particles);
	CvPoint estimateMeanState();
	void resampling();

	void printParticleInfoToFile(); 
	int isPositionExpAndFocusVisited(int indexOfParticle, int expLevel, int focusLevel);
};
