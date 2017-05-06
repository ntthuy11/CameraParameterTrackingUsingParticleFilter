#include "StdAfx.h"
#include "PF.h"

PF::PF(void) { }

PF::~PF(void) { 
	delete[] previousFrameHistogram;
	delete[] particles;
}

void PF::setParamsAndInit(int numParticles, double positionSigma, double velocitySigma, IplImage* initFrame, int initExpLevel, int initFocusLevel) { // giong constructor
	this->numParticles = numParticles;
	this->positionSigma = positionSigma;
	this->velocitySigma = velocitySigma;
	this->initExpLevel = initExpLevel;
	this->initFocusLevel = initFocusLevel;

	// init particles
	particles = new LevelParticle[numParticles];
	initParticles();

	// init previousFrameHistogram
	int* hist = new int[HIST_BINS];
	previousFrameHistogram = new double[HIST_BINS];
	for (int i = 0; i < HIST_BINS; i++) {
		hist[i] = 0;
		previousFrameHistogram[i] = .0;
	}

	IplImage* sobelDerivativeImg = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
	Util::sobelDerivative(initFrame, sobelDerivativeImg);

	Histogram::calculateGrayHist(initFrame, hist);							//Histogram::printIntHistToFile(L"..\\testdata\\intHist.txt", hist, HIST_BINS);
	Histogram::normalizeIntHist(hist, previousFrameHistogram, HIST_BINS);	//Histogram::printDoubleHistToFile(L"..\\testdata\\doubleHist.txt", previousFrameHistogram, HIST_BINS);
	
	// release
	delete[] hist;
	cvReleaseImage(&sobelDerivativeImg);
}

CvPoint PF::process(CString imgSetPath, int frameWidth, int frameHeight, int expLevelMax, int focusLevelMax) {	
	predict(expLevelMax, focusLevelMax);
	calculateParticlesInfo(imgSetPath, frameHeight, expLevelMax);
	printParticleInfoToFile(); // for debug

	CvPoint meanState = estimateMeanState();
	resampling();
	//printParticleInfoToFile(); // for debug

	return meanState;
}

// =============================================================================================

void PF::initParticles() {
	int factor = 1; // chia cho factor la de tranh truong hop ddu.ng bie^n

	double seed;
	srand((unsigned)time(NULL)); // dung de phat sinh so ngau nhien can cu theo clock cua CPU
	for(int i = 0; i < numParticles; i++){
		// init particles's Gaussian Random Number Generator, bo tao so ngau nhien nay dung de ta.o nhie^~u cho position cua particles
		seed = rand()*1.0/RAND_MAX;		particles[i].gaussianGenPositionExp.setSeed(seed, 0, positionSigma);
		seed = rand()*1.0/RAND_MAX;		particles[i].gaussianGenPositionFocus.setSeed(seed, 0, positionSigma);
		seed = rand()*1.0/RAND_MAX;		particles[i].gaussianGenVelocityExp.setSeed(seed, 0, velocitySigma);
		seed = rand()*1.0/RAND_MAX;		particles[i].gaussianGenVelocityFocus.setSeed(seed, 0, velocitySigma);

		// init particles' positions
		particles[i].positionExp	= initExpLevel + int( (particles[i].gaussianGenPositionExp.rnd() + 0.5) / factor ); 
		particles[i].positionFocus	= initFocusLevel + int( (particles[i].gaussianGenPositionFocus.rnd() + 0.5) / factor );
		particles[i].velocityExp	= int(particles[i].gaussianGenVelocityExp.rnd());
		particles[i].velocityFocus	= int(particles[i].gaussianGenVelocityFocus.rnd());

		// zero the weight
		particles[i].weight	= .0;
	}
}

void PF::predict(int expLevelMax, int focusLevelMax) {
	int maxRand = cvRound(this->positionSigma);
	int factor = 1;

	for (int i = 0; i < numParticles; i++) {
	
		// --- position ---
		particles[i].positionExp	+= particles[i].velocityExp + int( (particles[i].gaussianGenPositionExp.rnd() + 0.5) / factor );
		particles[i].positionFocus	+= particles[i].velocityFocus + int( (particles[i].gaussianGenPositionFocus.rnd() + 0.5) / factor );

		CvRNG rng_state = cvRNG(GetTickCount() + i);		
		if (particles[i].positionExp > expLevelMax) 		particles[i].positionExp = expLevelMax - cvRandInt(&rng_state) % maxRand; 
		if (particles[i].positionExp < 0)					particles[i].positionExp = cvRandInt(&rng_state) % maxRand; 
		if (particles[i].positionFocus > focusLevelMax)		particles[i].positionFocus = focusLevelMax - cvRandInt(&rng_state) % maxRand;
		if (particles[i].positionFocus < 0)					particles[i].positionFocus = cvRandInt(&rng_state) % maxRand;

		// --- velocity ---
		particles[i].velocityExp	+= int(particles[i].gaussianGenVelocityExp.rnd() + 0.5); // tinh lai!!!
		particles[i].velocityFocus	+= int(particles[i].gaussianGenVelocityFocus.rnd() + 0.5);
	}
}

void PF::calculateParticlesInfo(CString imgSetPath, int frameWidth, int frameHeight) {
	CString fn;

	int* hist = new int[HIST_BINS];
	double* currentFrameHistogram = new double[HIST_BINS];

	for (int i = 0; i < numParticles; i++) {
		int isVisited = isPositionExpAndFocusVisited(i, particles[i].positionExp, particles[i].positionFocus);

		if (isVisited != -1) { // particles[i] nay trung voi particles[isVisited] (ve positionExp va positionFocus)
			particles[i].weight = particles[isVisited].weight;
		} else {
			// reset histograms
			for (int j = 0; j < HIST_BINS; j++) {
				hist[j] = 0;
				currentFrameHistogram[j] = .0;
			}
			
			// load current frames
			CString imgSetPathCopy;		imgSetPathCopy.SetString(imgSetPath);
			fn.Format(L"%02d-%02d.bmp", particles[i].positionExp, particles[i].positionFocus);
			imgSetPathCopy.Append(fn);
			CStringA imgFilename (imgSetPathCopy);

			IplImage* currImg = cvCreateImage(cvSize(frameWidth, frameHeight), IPL_DEPTH_8U, 1);	currImg = cvLoadImage(imgFilename, 0); // force to load gray img (number 0)
			IplImage* sobelDerivativeImg = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
			Util::sobelDerivative(currImg, sobelDerivativeImg);

			// calculate histogram
			Histogram::calculateGrayHist(sobelDerivativeImg, hist);
			Histogram::normalizeIntHist(hist, currentFrameHistogram, HIST_BINS);

			// tinh weight cho moi particle
			double coeff = MathUtil::bhattacharyyaCoefficient(previousFrameHistogram, currentFrameHistogram);
			particles[i].weight = MathUtil::gaussianWeight(MathUtil::bhattacharyyaCoefficient(previousFrameHistogram, currentFrameHistogram), 0.5); // sigma = 0.5 co the thay doi

			cvReleaseImage(&currImg);
			cvReleaseImage(&sobelDerivativeImg);
		}
	}
	delete[] hist;
	delete[] currentFrameHistogram;

	normalizeWeights(particles);
}

void PF::normalizeWeights(LevelParticle* particles) {
	double sumWeights = 0;
	for (int i = 0; i < numParticles; i++) sumWeights += particles[i].weight;
	for (int i = 0; i < numParticles; i++) particles[i].weight /= sumWeights;
}

CvPoint PF::estimateMeanState() {
	double meanPositionExp = .0, meanPositionFocus = .0;
	for (int i = 0; i < numParticles; i++) {
		meanPositionExp += (particles[i].positionExp * particles[i].weight);
		meanPositionFocus += (particles[i].positionFocus * particles[i].weight);
	}
	return cvPoint(cvRound(meanPositionExp), cvRound(meanPositionFocus));
}

void PF::resampling() {
	LevelParticle* tmpParticles = new LevelParticle[numParticles];

	srand((unsigned)time(NULL)); // su dung truoc khi goi ham rand()
	CvRNG rng_state = cvRNG(rand());

	// ---- init cummulativeSumOfWeights ----
	double* cummulativeSumOfWeights = new double[numParticles];
	cummulativeSumOfWeights[0] = particles[0].weight;
	for (int i = 1; i < numParticles; i++) 
		cummulativeSumOfWeights[i] = cummulativeSumOfWeights[i-1] + particles[i].weight;

	// ---- main resampling ----
	int index = -1;
	for (int i = 0; i < numParticles; i++) {
		double r =  cvRandReal(&rng_state);
		for (int j = 0; j < numParticles; j++) {// search the smallest j for which c[j] > r
			if (cummulativeSumOfWeights[j] > r) {
				index = j;
				break;
			}
		}		
		tmpParticles[i].positionExp = particles[index].positionExp;
		tmpParticles[i].positionFocus = particles[index].positionFocus;
		tmpParticles[i].velocityExp = particles[index].velocityExp;
		tmpParticles[i].velocityFocus = particles[index].velocityFocus;

		// tra'o bo phat sinh so ngau nhien cu ==> de mang lai tinh dda da.ng cho viec phat sinh ngau nhien
		tmpParticles[i].gaussianGenPositionExp = particles[i].gaussianGenPositionExp;
		tmpParticles[i].gaussianGenPositionFocus = particles[i].gaussianGenPositionFocus;
		tmpParticles[i].gaussianGenVelocityExp = particles[i].gaussianGenVelocityExp;
		tmpParticles[i].gaussianGenVelocityFocus = particles[i].gaussianGenVelocityFocus;

		// zero the weights
		tmpParticles[i].weight = .0; // buoc phu. tro, khong that su can thiet
	}

	//
	delete[] cummulativeSumOfWeights;
	delete[] particles;
	particles = tmpParticles;
}

// ---------------------------------

void PF::printParticleInfoToFile() {
	CStdioFile f;	f.Open(L"..\\testdata\\particles.txt", CFile::modeCreate | CFile::modeWrite);
	CString text;

	f.WriteString(L"posExp        posFocus        velExp        velFocus        weight      \n");
	for (int i = 0; i < numParticles; i++) {
		text.Format(L"%2d          ", particles[i].positionExp);		f.WriteString(text);
		text.Format(L"%2d          ", particles[i].positionFocus);		f.WriteString(text);
		text.Format(L"%2d          ", particles[i].velocityExp);		f.WriteString(text);
		text.Format(L"%2d          ", particles[i].velocityFocus);		f.WriteString(text);
		text.Format(L"%2.5f \n", particles[i].weight);					f.WriteString(text);
	}
	f.Close();
}

// dung de tang toc do load anh. Nhung expLevel va focusLevel nao da xet roi thi ko phai load anh de xet lai, vi da tinh weight roi
// ham nay kiem tra co ton tai expLevel va focusLevel trong danh sach particles chua
int PF::isPositionExpAndFocusVisited(int indexOfParticle, int expLevel, int focusLevel) { 
	/*for (int i = 0; i < numParticles; i++) {
		if (particles[i].positionExp == expLevel && particles[i].positionFocus == focusLevel 
			&& particles[i].weight != 0) // weight != 0 de co the su dung weight o particles[i] nay
			return i;
	}
	return -1;*/

	// co the dung cach nay de khong phai xet het particles[] (trong truong hop tim khong thay)
	// vi chac chan la nhung particle of index < indexOfParticle la weight deu da duoc tinh
	for (int i = 0; i < indexOfParticle; i++) {
		if (particles[i].positionExp == expLevel && particles[i].positionFocus == focusLevel)
			return i;
	}
	return -1;
}