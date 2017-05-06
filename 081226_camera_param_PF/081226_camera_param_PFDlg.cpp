// 081226_camera_param_PFDlg.cpp : implementation file
//

#include "stdafx.h"
#include "081226_camera_param_PF.h"
#include "081226_camera_param_PFDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ---------------------------------
#define LEVEL_APERTURE_MAX	(15)	// level tot nhat de detect canh

#define N_LEVEL_EXPOSURE	(15)
#define N_LEVEL_FOCUS		(14)
#define TOTAL_LEVELS		(210)	// N_LEVEL_EXPOSURE * N_LEVEL_FOCUS

#define LOCAL_MAXIMA_WIN_HEIGHT		(5)
#define LOCAL_MAXIMA_WIN_WIDTH		(5)
#define LOCAL_MAXIMA_CENTER_EXP		(0)
#define LOCAL_MAXIMA_CENTER_FOCUS	(0)

#define LOCAL_MAXIMA_RANDOM_NUM		(20)

#define PF_INIT_EXP					(13)
#define PF_INIT_FOCUS				(13)
#define PF_NUM						(100)
#define PF_POSITION_SIGMA			(3.0)
#define PF_VELOCITY_SIGMA			(1.0)

#define SHT_THRESHOLD				(100)

Camera camera;
PF pf;
IplImage *capturedImg, *sobelDerivativeImg, *previousFrame, *edgeImg, *lineImg;
CString imgSetPath("");

int expLevel_max, focusLevel_max;
double maxEntropy, maxEnergy;

int localMaxWinH, localMaxWinW, localMaxCenterExp, localMaxCenterFocus, localMaxRandomNum;
int pfInitExp, pfInitFocus, pfNum;
double pfPositionSigma, pfVelocitySigma;
CvPoint pfResult;

StandardHough stdHough;
int shtThreshold;
int nDetectedLines;

unsigned long timeDiff_captureImg, timeDiff_measureEntropy, timeDiff_measureEnergyLP, timeDiff_pf;
__int64 freq, tStart, tStop;

char filenameInMultiByte[256];

// ---------------------------------

CMy081226_camera_param_PFDlg::CMy081226_camera_param_PFDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMy081226_camera_param_PFDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMy081226_camera_param_PFDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMy081226_camera_param_PFDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_QUIT, &CMy081226_camera_param_PFDlg::OnBnClickedButtonQuit)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE_IMG, &CMy081226_camera_param_PFDlg::OnBnClickedButtonCaptureImg)
	ON_BN_CLICKED(IDC_BUTTON_MEASURE_ENTROPY_SCAN_ALL, &CMy081226_camera_param_PFDlg::OnBnClickedButtonMeasureEntropyScanAll)
	ON_BN_CLICKED(IDC_BUTTON_MEASURE_ENTROPY_LOCAL_MAXIMA, &CMy081226_camera_param_PFDlg::OnBnClickedButtonMeasureEntropyLocalMaxima)
	ON_BN_CLICKED(IDC_BUTTON_MEASURE_ENTROPY_LOCAL_MAXIMA2, &CMy081226_camera_param_PFDlg::OnBnClickedButtonMeasureEntropyLocalMaxima2)
	ON_BN_CLICKED(IDC_BUTTON_PF_RUN, &CMy081226_camera_param_PFDlg::OnBnClickedButtonPfRun)
	ON_BN_CLICKED(IDC_BUTTON_ENERGY_LP_SCAN_ALL, &CMy081226_camera_param_PFDlg::OnBnClickedButtonEnergyLpScanAll)
	ON_BN_CLICKED(IDC_BUTTON_TOOL_LOAD_IMG, &CMy081226_camera_param_PFDlg::OnBnClickedButtonToolLoadImg)
	ON_BN_CLICKED(IDC_BUTTON_TOOL_EDGE_DETECT, &CMy081226_camera_param_PFDlg::OnBnClickedButtonToolEdgeDetect)
	ON_BN_CLICKED(IDC_BUTTON_TOOL_LINE_DETECT, &CMy081226_camera_param_PFDlg::OnBnClickedButtonToolLineDetect)
END_MESSAGE_MAP()


// CMy081226_camera_param_PFDlg message handlers

BOOL CMy081226_camera_param_PFDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	initGUI();
	initVariables();


	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMy081226_camera_param_PFDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMy081226_camera_param_PFDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// ===========================================================================================

void CMy081226_camera_param_PFDlg::OnBnClickedButtonQuit() {
	releaseResoures();
	OnOK();
}

void CMy081226_camera_param_PFDlg::OnBnClickedButtonCaptureImg() {
	int stopExpLevel = 1, stopFocusLevel = 1;
	if (isCheckExposure())	stopExpLevel = N_LEVEL_EXPOSURE;
	if (isCheckFocus())		stopFocusLevel = N_LEVEL_FOCUS;	

	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);	QueryPerformanceCounter((LARGE_INTEGER*)&tStart); // tick count START

	camera.changeAperture(LEVEL_APERTURE_MAX);
	for (int expLevel = 0; expLevel < stopExpLevel; expLevel++) {
		camera.changeExposure(expLevel);
		for (int focusLevel = 0; focusLevel < stopFocusLevel; focusLevel++) {
			camera.changeFocus(focusLevel);
			captureFrame(capturedImg);

			CString s;			s.Format(L"..\\testdata\\%02d-%02d.bmp", expLevel, focusLevel);
			CStringA fn (s);	cvSaveImage(fn, capturedImg);
		}
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&tStop);	timeDiff_captureImg = (unsigned long)((tStop - tStart) * 1000 / freq); // tick count STOP
	updateResult();
}

void CMy081226_camera_param_PFDlg::OnBnClickedButtonMeasureEntropyScanAll() {
	getInput();
	CString fn;	
	int count = 0;
	double* imgEntropies = new double[TOTAL_LEVELS];	

	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);	QueryPerformanceCounter((LARGE_INTEGER*)&tStart); // tick count START	
	for (int expLevel = 0; expLevel < N_LEVEL_EXPOSURE; expLevel++) {
		for (int focusLevel = 0; focusLevel < N_LEVEL_FOCUS; focusLevel++) {
			CString imgSetPathCopy;		imgSetPathCopy.SetString(imgSetPath);
			if (isCheckImgSetPrevious()) fn.Format(L"previousFrame\\%02d-%02d.bmp", expLevel, focusLevel);
			else fn.Format(L"%02d-%02d.bmp", expLevel, focusLevel);
			imgSetPathCopy.Append(fn);
			CStringA imgFilename (imgSetPathCopy);

			cvReleaseImage(&capturedImg);					capturedImg = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
			cvReleaseImage(&sobelDerivativeImg);			sobelDerivativeImg = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
			capturedImg = cvLoadImage(imgFilename, 0); // force to load gray img (number 0)	
			Util::sobelDerivative(capturedImg, sobelDerivativeImg);

			imgEntropies[count] = Histogram::calculateEntropy(sobelDerivativeImg); // calculate entropy of first derivative image			
			if (maxEntropy < imgEntropies[count]) {
				maxEntropy = imgEntropies[count];
				expLevel_max = expLevel;
				focusLevel_max = focusLevel;
			}
			count++;
		}
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&tStop);	timeDiff_measureEntropy = (unsigned long)((tStop - tStart) * 1000 / freq); // tick count STOP
	updateResult();
	//saveEntropyArrayToFile(L"..\\testdata\\entropy_scanAll.txt", imgEntropies, 0, N_LEVEL_EXPOSURE, 0, N_LEVEL_FOCUS);

	// release
	delete[] imgEntropies;
	imgSetPath.Empty();
	expLevel_max = focusLevel_max = -1;
	maxEntropy = -1.0;
}

void CMy081226_camera_param_PFDlg::OnBnClickedButtonMeasureEntropyLocalMaxima() { // height ~ exposure, width ~ focus
	getInput();

	CString fn;	
	int halfWinH = localMaxWinH / 2, halfWinW = localMaxWinW / 2;
	int startExpLevel = localMaxCenterExp - halfWinH;		if (startExpLevel < 0) startExpLevel = 0;
	int endExpLevel = localMaxCenterExp + halfWinH;			if (endExpLevel >= N_LEVEL_EXPOSURE) endExpLevel = N_LEVEL_EXPOSURE - 1;
	int startFocusLevel = localMaxCenterFocus - halfWinW;	if (startFocusLevel < 0) startFocusLevel = 0;
	int	endFocusLevel = localMaxCenterFocus + halfWinW;		if (endFocusLevel >= N_LEVEL_FOCUS) endFocusLevel = N_LEVEL_FOCUS - 1;
	int count = 0;

	double* imgEntropies = new double[(endExpLevel - startExpLevel + 1) * (endFocusLevel - startFocusLevel + 1)];

	// tranh truong hop cua so cha(~n  ==> cong them 1 de thanh cua so le~

	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);	QueryPerformanceCounter((LARGE_INTEGER*)&tStart); // tick count START
	for (int expLevel = startExpLevel; expLevel <= endExpLevel; expLevel++) {
		for (int focusLevel = startFocusLevel; focusLevel <= endFocusLevel; focusLevel++) {
			CString imgSetPathCopy;		imgSetPathCopy.SetString(imgSetPath);
			if (isCheckImgSetPrevious()) fn.Format(L"previousFrame\\%02d-%02d.bmp", expLevel, focusLevel);
			else fn.Format(L"%02d-%02d.bmp", expLevel, focusLevel);
			imgSetPathCopy.Append(fn);
			CStringA imgFilename (imgSetPathCopy);

			cvReleaseImage(&capturedImg);					capturedImg = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
			cvReleaseImage(&sobelDerivativeImg);			sobelDerivativeImg = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
			capturedImg = cvLoadImage(imgFilename, 0); // force to load gray img (number 0)	
			Util::sobelDerivative(capturedImg, sobelDerivativeImg);

			imgEntropies[count] = Histogram::calculateEntropy(sobelDerivativeImg); // calculate entropy of first derivative image			
			if (maxEntropy < imgEntropies[count]) {
				maxEntropy = imgEntropies[count];
				expLevel_max = expLevel;
				focusLevel_max = focusLevel;
			}
			count++;
		}
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&tStop);	timeDiff_measureEntropy = (unsigned long)((tStop - tStart) * 1000 / freq); // tick count STOP
	updateResult();
	saveEntropyArrayToFile(L"..\\testdata\\entropy_localMax.txt", imgEntropies, startExpLevel, endExpLevel + 1, startFocusLevel, endFocusLevel + 1);

	// release
	delete[] imgEntropies;
	imgSetPath.Empty();
	expLevel_max = focusLevel_max = -1;
	maxEntropy = -1.0;
}

void CMy081226_camera_param_PFDlg::OnBnClickedButtonMeasureEntropyLocalMaxima2() { // local maxima - random
	getInput();

	CString fn;	
	int halfWinH = localMaxWinH / 2, halfWinW = localMaxWinW / 2;
	int startExpLevel = localMaxCenterExp - halfWinH;		if (startExpLevel < 0) startExpLevel = 0;
	int endExpLevel = localMaxCenterExp + halfWinH;			if (endExpLevel >= N_LEVEL_EXPOSURE) endExpLevel = N_LEVEL_EXPOSURE - 1;
	int startFocusLevel = localMaxCenterFocus - halfWinW;	if (startFocusLevel < 0) startFocusLevel = 0;
	int	endFocusLevel = localMaxCenterFocus + halfWinW;		if (endFocusLevel >= N_LEVEL_FOCUS) endFocusLevel = N_LEVEL_FOCUS - 1;

	// neu so lan lay random lon hon kich thuoc cua so quet thi chinh lai cho ba(`ng
	int nLevels = (endExpLevel - startExpLevel + 1) * (endFocusLevel - startFocusLevel + 1);
	if (localMaxRandomNum > nLevels) localMaxRandomNum = nLevels; 

	int* randNumForExp = new int[localMaxRandomNum];		Util::generateRandomNumbers(localMaxRandomNum, endExpLevel - startExpLevel + 1, randNumForExp);
	int* randNumForFocus = new int[localMaxRandomNum];		Util::generateRandomNumbers(localMaxRandomNum, endFocusLevel - startFocusLevel + 1, randNumForFocus);
	double* imgEntropies = new double[localMaxRandomNum];

	// main
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);	QueryPerformanceCounter((LARGE_INTEGER*)&tStart); // tick count START
	for (int i = 0; i < localMaxRandomNum; i++) {
		int expLevel = startExpLevel + randNumForExp[i];
		int focusLevel = startFocusLevel + randNumForFocus[i];

		CString imgSetPathCopy;		imgSetPathCopy.SetString(imgSetPath);
		if (isCheckImgSetPrevious()) fn.Format(L"previousFrame\\%02d-%02d.bmp", expLevel, focusLevel);
		else fn.Format(L"%02d-%02d.bmp", expLevel, focusLevel);
		imgSetPathCopy.Append(fn);
		CStringA imgFilename (imgSetPathCopy);

		cvReleaseImage(&capturedImg);					capturedImg = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
		cvReleaseImage(&sobelDerivativeImg);			sobelDerivativeImg = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
		capturedImg = cvLoadImage(imgFilename, 0); // force to load gray img (number 0)	
		Util::sobelDerivative(capturedImg, sobelDerivativeImg);

		imgEntropies[i] = Histogram::calculateEntropy(sobelDerivativeImg); // calculate entropy of first derivative image		
		if (maxEntropy < imgEntropies[i]) {
			maxEntropy = imgEntropies[i];
			expLevel_max = expLevel;
			focusLevel_max = focusLevel;
		}
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&tStop);	timeDiff_measureEntropy = (unsigned long)((tStop - tStart) * 1000 / freq); // tick count STOP
	updateResult();
	
	// saveEntropyArrayToFile
	CStdioFile f;	f.Open(L"..\\testdata\\entropy_localMaxRandom.txt", CFile::modeCreate | CFile::modeWrite);
	CString text;
	for (int i = 0; i < localMaxRandomNum; i++) {
		text.Format(L"%02d_%02d    %5.3f\n", startExpLevel + randNumForExp[i], startFocusLevel + randNumForFocus[i], imgEntropies[i]);
		f.WriteString(text);
	}
	f.Close();

	// release
	delete[] imgEntropies;
	imgSetPath.Empty();
	expLevel_max = focusLevel_max = -1;
	maxEntropy = -1.0;
}

void CMy081226_camera_param_PFDlg::OnBnClickedButtonEnergyLpScanAll() {
	getInput();
	CString fn;	
	int count = 0;
	double* energies = new double[TOTAL_LEVELS];	

	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);	QueryPerformanceCounter((LARGE_INTEGER*)&tStart); // tick count START	
	for (int expLevel = 0; expLevel < N_LEVEL_EXPOSURE; expLevel++) {
		for (int focusLevel = 0; focusLevel < N_LEVEL_FOCUS; focusLevel++) {
			CString imgSetPathCopy;		imgSetPathCopy.SetString(imgSetPath);
			if (isCheckImgSetPrevious()) fn.Format(L"previousFrame\\%02d-%02d.bmp", expLevel, focusLevel);
			else fn.Format(L"%02d-%02d.bmp", expLevel, focusLevel);
			imgSetPathCopy.Append(fn);
			CStringA imgFilename (imgSetPathCopy);

			cvReleaseImage(&capturedImg);					capturedImg = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
			capturedImg = cvLoadImage(imgFilename, 0); // force to load gray img (number 0)	
			

			energies[count] = Util::energyOfLowPassFilteredImageGradient(capturedImg, 3);
			if (maxEnergy < energies[count]) {
				maxEnergy = energies[count];
				expLevel_max = expLevel;
				focusLevel_max = focusLevel;
			}
			count++;
		}
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&tStop);	timeDiff_measureEnergyLP = (unsigned long)((tStop - tStart) * 1000 / freq); // tick count STOP
	updateResult();
	saveEntropyArrayToFile(L"..\\testdata\\energy_scanAll.txt", energies, 0, N_LEVEL_EXPOSURE, 0, N_LEVEL_FOCUS);

	// release
	delete[] energies;
	imgSetPath.Empty();
	expLevel_max = focusLevel_max = -1;
	maxEnergy = -1.0;
}

void CMy081226_camera_param_PFDlg::OnBnClickedButtonPfRun() {
	getInput();

	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);	QueryPerformanceCounter((LARGE_INTEGER*)&tStart); // tick count START

	// load previous frame
	CString fn;	
	CString imgSetPathCopy;		imgSetPathCopy.SetString(imgSetPath);
	fn.Format(L"previousFrame\\%02d-%02d.bmp", pfInitExp, pfInitFocus);
	imgSetPathCopy.Append(fn);
	CStringA imgFilename (imgSetPathCopy);
	previousFrame = cvLoadImage(imgFilename, 0); // force to load gray img (number 0)
	
	//
	pf.setParamsAndInit(pfNum, pfPositionSigma, pfVelocitySigma, previousFrame, pfInitExp, pfInitFocus);
	pfResult = pf.process(imgSetPath, FRAME_WIDTH, FRAME_HEIGHT, N_LEVEL_EXPOSURE - 1,  N_LEVEL_FOCUS - 1);

	QueryPerformanceCounter((LARGE_INTEGER*)&tStop);	timeDiff_pf = (unsigned long)((tStop - tStart) * 1000 / freq); // tick count STOP
	updateResult();

	//
	imgSetPath.Empty();
}

void CMy081226_camera_param_PFDlg::OnBnClickedButtonToolLoadImg() {
	CFileDialog dlg(TRUE, _T("*.avi"), _T(""), OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,_T("Image (*.bmp)|*.bmp|All Files (*.*)|*.*||"),NULL);	
	if (dlg.DoModal() != IDOK) return;		
	WideCharToMultiByte(CP_ACP, 0, dlg.GetPathName(), -1, filenameInMultiByte, 256, NULL, NULL);	
	
	capturedImg = cvLoadImage(filenameInMultiByte, 0); // force to load gray img (number 0)	
	cvNamedWindow("Img");
	cvShowImage("Img", capturedImg);
}

void CMy081226_camera_param_PFDlg::OnBnClickedButtonToolEdgeDetect() {
	cvCanny(capturedImg, edgeImg, 50, 200);
	cvNamedWindow("Edge Img");
	cvShowImage("Edge Img", edgeImg);

	//CString s;			s += filenameInMultiByte;	s += "_edge.bmp";
	//CStringA fn (s);	cvSaveImage(fn, edgeImg);
}

void CMy081226_camera_param_PFDlg::OnBnClickedButtonToolLineDetect() {
	OnBnClickedButtonToolEdgeDetect();

	getInput();

	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* result = cvCreateSeq(CV_SEQ_ELTYPE_GENERIC, sizeof(CvSeq), sizeof(CvRect), storage);
	stdHough.run(edgeImg, result, 1, 0.01, shtThreshold, 100);
	
	cvCvtColor(capturedImg, lineImg, CV_GRAY2BGR);
	nDetectedLines = result->total;
	for(int i = 0; i < nDetectedLines; i++) {	
		CvRect* lr = (CvRect*)cvGetSeqElem(result,i);
		CvPoint startPoint = { lr->x, lr->y };
		CvPoint endPoint = { lr->width, lr->height };
		cvLine(lineImg, startPoint, endPoint, CV_RGB(255, 0, 0), 1);
	}
	cvNamedWindow("Line Img");
	cvShowImage("Line Img", lineImg);

	updateResult();
}

// -----------------------------------------------------------

void CMy081226_camera_param_PFDlg::initGUI() {

	// capture images
	((CButton *) this->GetDlgItem(IDC_CHECK_CAPTURE_IMG_IS_EXPOSURE))->SetCheck(BST_CHECKED);
	((CButton *) this->GetDlgItem(IDC_CHECK_CAPTURE_IMG_IS_FOCUS))->SetCheck(BST_CHECKED);

	// select image set
	CComboBox *combo;
	combo = (CComboBox *) this->GetDlgItem(IDC_COMBO_IMG_SET);
	combo->AddString(L"090112_1normal_1-2"); // 0		
	combo->AddString(L"090112_1normal_2-3"); // 1
	combo->AddString(L"090112_1normal_3-4"); // 2
	combo->AddString(L"090112_1normal_4-5"); // 3

	combo->AddString(L"090116_pantilt_1-2"); // 4
	combo->AddString(L"090116_pantilt_2-3"); // 5
	combo->AddString(L"090116_pantilt_3-4"); // 6
	combo->AddString(L"090116_pantilt_4-5"); // 7
	combo->AddString(L"090116_pantilt_5-6"); // 8
	combo->AddString(L"090116_pantilt_6-7"); // 9
	combo->AddString(L"090116_pantilt_7-8"); // 10
	combo->AddString(L"090116_pantilt_8-9"); // 11
	combo->AddString(L"090116_pantilt_9-10"); // 12

	combo->AddString(L"090117_overexposure_1-2"); // 13
	combo->AddString(L"090117_overexposure_2-3"); // 14
	combo->AddString(L"090117_overexposure_3-4"); // 15
	combo->AddString(L"090117_overexposure_4-5"); // 16
	combo->AddString(L"090117_overexposure_5-6"); // 17
	combo->AddString(L"090117_overexposure_6-7"); // 18
	combo->AddString(L"090117_overexposure_7-8"); // 19
	combo->AddString(L"090117_overexposure_8-9"); // 20
	combo->AddString(L"090117_overexposure_9-10"); // 21

	combo->AddString(L"090203_testfocus_1-2"); // 22
	combo->AddString(L"090203_testfocus_2-3"); // 23
	combo->AddString(L"090203_testfocus_3-4"); // 24
	combo->AddString(L"090203_testfocus_4-5"); // 25
	combo->AddString(L"090203_testfocus_5-6"); // 26
	combo->AddString(L"090203_testfocus_6-7"); // 27
	combo->AddString(L"090203_testfocus_7-8"); // 28
	combo->AddString(L"090203_testfocus_8-9"); // 29
	combo->AddString(L"090203_testfocus_9-10"); // 30

	combo->AddString(L"090203z_pantitl_exp_1-2"); // 31
	combo->AddString(L"090203z_pantitl_exp_2-3"); // 32
	combo->AddString(L"090203z_pantitl_exp_3-4"); // 33
	combo->AddString(L"090203z_pantitl_exp_4-5"); // 34
	combo->AddString(L"090203z_pantitl_exp_5-6"); // 35
	combo->AddString(L"090203z_pantitl_exp_6-7"); // 36
	combo->AddString(L"090203z_pantitl_exp_7-8"); // 37
	combo->AddString(L"090203z_pantitl_exp_8-9"); // 38
	combo->AddString(L"090203z_pantitl_exp_9-10"); // 39

	combo->AddString(L"090204_testfocus_exp_1-2"); // 40
	combo->AddString(L"090204_testfocus_exp_2-3"); // 41
	combo->AddString(L"090204_testfocus_exp_3-4"); // 42
	combo->AddString(L"090204_testfocus_exp_4-5"); // 43
	combo->AddString(L"090204_testfocus_exp_5-6"); // 44
	combo->AddString(L"090204_testfocus_exp_6-7"); // 45
	combo->AddString(L"090204_testfocus_exp_7-8"); // 46
	combo->AddString(L"090204_testfocus_exp_8-9"); // 47
	combo->AddString(L"090204_testfocus_exp_9-10"); // 48

	combo->SetCurSel(31);


	((CButton *) this->GetDlgItem(IDC_CHECK_IMG_SET_PREVIOUS))->SetCheck(BST_CHECKED);

	// measure using entropy - local maxima
	CString s;
	s.Format(L"%2d", LOCAL_MAXIMA_WIN_HEIGHT);		this->GetDlgItem(IDC_EDIT_MEASURE_ENTROPY_LOCAL_MAXIMA_WIN_HEIGHT)->SetWindowTextW((LPCTSTR)s); 
	s.Format(L"%2d", LOCAL_MAXIMA_WIN_WIDTH);		this->GetDlgItem(IDC_EDIT_MEASURE_ENTROPY_LOCAL_MAXIMA_WIN_WIDTH)->SetWindowTextW((LPCTSTR)s); 
	s.Format(L"%2d", LOCAL_MAXIMA_CENTER_EXP);		this->GetDlgItem(IDC_EDIT_MEASURE_ENTROPY_LOCAL_MAXIMA_CENTER_EXP)->SetWindowTextW((LPCTSTR)s); 
	s.Format(L"%2d", LOCAL_MAXIMA_CENTER_FOCUS);	this->GetDlgItem(IDC_EDIT_MEASURE_ENTROPY_LOCAL_MAXIMA_CENTER_FOCUS)->SetWindowTextW((LPCTSTR)s); 	
	s.Format(L"%3d", LOCAL_MAXIMA_RANDOM_NUM);		this->GetDlgItem(IDC_EDIT_MEASURE_ENTROPY_LOCAL_MAXIMA_RANDOM_NUM)->SetWindowTextW((LPCTSTR)s); 

	// PF
	s.Format(L"%2d", PF_INIT_EXP);			this->GetDlgItem(IDC_EDIT_PF_INIT_EXP)->SetWindowTextW((LPCTSTR)s); 
	s.Format(L"%2d", PF_INIT_FOCUS);		this->GetDlgItem(IDC_EDIT_PF_INIT_FOCUS)->SetWindowTextW((LPCTSTR)s); 
	s.Format(L"%3d", PF_NUM);				this->GetDlgItem(IDC_EDIT_PF_NUM)->SetWindowTextW((LPCTSTR)s); 
	s.Format(L"%2.2f", PF_POSITION_SIGMA);	this->GetDlgItem(IDC_EDIT_PF_POSITION_SIGMA)->SetWindowTextW((LPCTSTR)s); 
	s.Format(L"%2.2f", PF_VELOCITY_SIGMA);	this->GetDlgItem(IDC_EDIT_PF_VELOCITY_SIGMA)->SetWindowTextW((LPCTSTR)s); 

	// TOOLS
	s.Format(L"%3d", SHT_THRESHOLD);		this->GetDlgItem(IDC_EDIT_TOOL_LINE_DETECT_THR)->SetWindowTextW((LPCTSTR)s); 
}

void CMy081226_camera_param_PFDlg::initVariables() {	
	capturedImg			=	cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
	sobelDerivativeImg	=	cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
	previousFrame		=	cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
	edgeImg				=	cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
	lineImg				=	cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 3);

	timeDiff_captureImg = timeDiff_measureEntropy = timeDiff_pf = timeDiff_measureEnergyLP = -1;	
	expLevel_max = focusLevel_max = -1;
	maxEntropy = maxEnergy = -1.0;

	// measure using entropy - local maxima
	localMaxWinH = LOCAL_MAXIMA_WIN_HEIGHT;
	localMaxWinW = LOCAL_MAXIMA_WIN_WIDTH;
	localMaxCenterExp = LOCAL_MAXIMA_CENTER_EXP;
	localMaxCenterFocus = LOCAL_MAXIMA_CENTER_FOCUS;
	localMaxRandomNum = LOCAL_MAXIMA_RANDOM_NUM;

	// PF
	pfInitExp = PF_INIT_EXP;
	pfInitFocus = PF_INIT_FOCUS;
	pfNum = PF_NUM;
	pfPositionSigma = PF_POSITION_SIGMA;
	pfVelocitySigma = PF_VELOCITY_SIGMA;

	// TOOLS
	shtThreshold = SHT_THRESHOLD;
	nDetectedLines = -1;
}

void CMy081226_camera_param_PFDlg::releaseResoures() {
	cvDestroyAllWindows();

	cvReleaseImage(&capturedImg);	
	cvReleaseImage(&sobelDerivativeImg);
	cvReleaseImage(&previousFrame);
	cvReleaseImage(&edgeImg);
	cvReleaseImage(&lineImg);
}

void CMy081226_camera_param_PFDlg::getInput() {
	
	// measure using entropy - select image set
	switch (((CComboBox *) GetDlgItem(IDC_COMBO_IMG_SET))->GetCurSel()) {
		case 0:		imgSetPath.Append(L"..\\testdata\\090112_1normal_1-2\\");	break;
		case 1:		imgSetPath.Append(L"..\\testdata\\090112_1normal_2-3\\");	break;
		case 2:		imgSetPath.Append(L"..\\testdata\\090112_1normal_3-4\\");	break;
		case 3:		imgSetPath.Append(L"..\\testdata\\090112_1normal_4-5\\");	break;

		case 4:		imgSetPath.Append(L"..\\testdata\\090116_pantilt_1-2\\");	break;
		case 5:		imgSetPath.Append(L"..\\testdata\\090116_pantilt_2-3\\");	break;
		case 6:		imgSetPath.Append(L"..\\testdata\\090116_pantilt_3-4\\");	break;
		case 7:		imgSetPath.Append(L"..\\testdata\\090116_pantilt_4-5\\");	break;
		case 8:		imgSetPath.Append(L"..\\testdata\\090116_pantilt_5-6\\");	break;
		case 9:		imgSetPath.Append(L"..\\testdata\\090116_pantilt_6-7\\");	break;
		case 10:	imgSetPath.Append(L"..\\testdata\\090116_pantilt_7-8\\");	break;
		case 11:	imgSetPath.Append(L"..\\testdata\\090116_pantilt_8-9\\");	break;
		case 12:	imgSetPath.Append(L"..\\testdata\\090116_pantilt_9-10\\");	break;

		case 13:	imgSetPath.Append(L"..\\testdata\\090117_overexposure_1-2\\");	break;
		case 14:	imgSetPath.Append(L"..\\testdata\\090117_overexposure_2-3\\");	break;
		case 15:	imgSetPath.Append(L"..\\testdata\\090117_overexposure_3-4\\");	break;
		case 16:	imgSetPath.Append(L"..\\testdata\\090117_overexposure_4-5\\");	break;
		case 17:	imgSetPath.Append(L"..\\testdata\\090117_overexposure_5-6\\");	break;
		case 18:	imgSetPath.Append(L"..\\testdata\\090117_overexposure_6-7\\");	break;
		case 19:	imgSetPath.Append(L"..\\testdata\\090117_overexposure_7-8\\");	break;
		case 20:	imgSetPath.Append(L"..\\testdata\\090117_overexposure_8-9\\");	break;
		case 21:	imgSetPath.Append(L"..\\testdata\\090117_overexposure_9-10\\");	break;

		case 22:	imgSetPath.Append(L"..\\testdata\\090203_testfocus_1-2\\");	break;
		case 23:	imgSetPath.Append(L"..\\testdata\\090203_testfocus_2-3\\");	break;
		case 24:	imgSetPath.Append(L"..\\testdata\\090203_testfocus_3-4\\");	break;
		case 25:	imgSetPath.Append(L"..\\testdata\\090203_testfocus_4-5\\");	break;
		case 26:	imgSetPath.Append(L"..\\testdata\\090203_testfocus_5-6\\");	break;
		case 27:	imgSetPath.Append(L"..\\testdata\\090203_testfocus_6-7\\");	break;
		case 28:	imgSetPath.Append(L"..\\testdata\\090203_testfocus_7-8\\");	break;
		case 29:	imgSetPath.Append(L"..\\testdata\\090203_testfocus_8-9\\");	break;
		case 30:	imgSetPath.Append(L"..\\testdata\\090203_testfocus_9-10\\");break;

		case 31:	imgSetPath.Append(L"..\\testdata\\090203z_pantitl_exp_1-2\\");	break;
		case 32:	imgSetPath.Append(L"..\\testdata\\090203z_pantitl_exp_2-3\\");	break;
		case 33:	imgSetPath.Append(L"..\\testdata\\090203z_pantitl_exp_3-4\\");	break;
		case 34:	imgSetPath.Append(L"..\\testdata\\090203z_pantitl_exp_4-5\\");	break;
		case 35:	imgSetPath.Append(L"..\\testdata\\090203z_pantitl_exp_5-6\\");	break;
		case 36:	imgSetPath.Append(L"..\\testdata\\090203z_pantitl_exp_6-7\\");	break;
		case 37:	imgSetPath.Append(L"..\\testdata\\090203z_pantitl_exp_7-8\\");	break;
		case 38:	imgSetPath.Append(L"..\\testdata\\090203z_pantitl_exp_8-9\\");	break;
		case 39:	imgSetPath.Append(L"..\\testdata\\090203z_pantitl_exp_9-10\\");	break;

		case 40:	imgSetPath.Append(L"..\\testdata\\090204_testfocus_exp_1-2\\");	break;
		case 41:	imgSetPath.Append(L"..\\testdata\\090204_testfocus_exp_2-3\\");	break;
		case 42:	imgSetPath.Append(L"..\\testdata\\090204_testfocus_exp_3-4\\");	break;
		case 43:	imgSetPath.Append(L"..\\testdata\\090204_testfocus_exp_4-5\\");	break;
		case 44:	imgSetPath.Append(L"..\\testdata\\090204_testfocus_exp_5-6\\");	break;
		case 45:	imgSetPath.Append(L"..\\testdata\\090204_testfocus_exp_6-7\\");	break;
		case 46:	imgSetPath.Append(L"..\\testdata\\090204_testfocus_exp_7-8\\");	break;
		case 47:	imgSetPath.Append(L"..\\testdata\\090204_testfocus_exp_8-9\\");	break;
		case 48:	imgSetPath.Append(L"..\\testdata\\090204_testfocus_exp_9-10\\");break;
	}

	// measure using entropy - local maxima
	CString localMaxWinHString, localMaxWinWString, localMaxCenterExpString, localMaxCenterFocusString, localMaxRandomNumString;
	this->GetDlgItem(IDC_EDIT_MEASURE_ENTROPY_LOCAL_MAXIMA_WIN_HEIGHT)->GetWindowTextW(localMaxWinHString);		
	this->GetDlgItem(IDC_EDIT_MEASURE_ENTROPY_LOCAL_MAXIMA_WIN_WIDTH)->GetWindowTextW(localMaxWinWString);
	this->GetDlgItem(IDC_EDIT_MEASURE_ENTROPY_LOCAL_MAXIMA_CENTER_EXP)->GetWindowTextW(localMaxCenterExpString);
	this->GetDlgItem(IDC_EDIT_MEASURE_ENTROPY_LOCAL_MAXIMA_CENTER_FOCUS)->GetWindowTextW(localMaxCenterFocusString);
	this->GetDlgItem(IDC_EDIT_MEASURE_ENTROPY_LOCAL_MAXIMA_RANDOM_NUM)->GetWindowTextW(localMaxRandomNumString);
	if (!localMaxWinHString.IsEmpty())			localMaxWinH = (int)wcstod(localMaxWinHString, NULL);
	if (!localMaxWinWString.IsEmpty())			localMaxWinW = (int)wcstod(localMaxWinWString, NULL);
	if (!localMaxCenterExpString.IsEmpty())		localMaxCenterExp = (int)wcstod(localMaxCenterExpString, NULL);
	if (!localMaxCenterFocusString.IsEmpty())	localMaxCenterFocus = (int)wcstod(localMaxCenterFocusString, NULL);
	if (!localMaxRandomNumString.IsEmpty())		localMaxRandomNum = (int)wcstod(localMaxRandomNumString, NULL);

	// PF
	CString pfInitExpString, pfInitFocusString, pfNumString, pfPositionSigmaString, pfVelocitySigmaString;
	this->GetDlgItem(IDC_EDIT_PF_INIT_EXP)->GetWindowTextW(pfInitExpString);
	this->GetDlgItem(IDC_EDIT_PF_INIT_FOCUS)->GetWindowTextW(pfInitFocusString);
	this->GetDlgItem(IDC_EDIT_PF_NUM)->GetWindowTextW(pfNumString);
	this->GetDlgItem(IDC_EDIT_PF_POSITION_SIGMA)->GetWindowTextW(pfPositionSigmaString);
	this->GetDlgItem(IDC_EDIT_PF_VELOCITY_SIGMA)->GetWindowTextW(pfVelocitySigmaString);
	if (!pfInitExpString.IsEmpty())			pfInitExp = (int)wcstod(pfInitExpString, NULL);
	if (!pfInitFocusString.IsEmpty())		pfInitFocus = (int)wcstod(pfInitFocusString, NULL);
	if (!pfNumString.IsEmpty())				pfNum = (int)wcstod(pfNumString, NULL);
	if (!pfPositionSigmaString.IsEmpty())	pfPositionSigma = wcstod(pfPositionSigmaString, NULL);
	if (!pfVelocitySigmaString.IsEmpty())	pfVelocitySigma = wcstod(pfVelocitySigmaString, NULL);

	// TOOLS
	CString shtThresholdString;
	this->GetDlgItem(IDC_EDIT_TOOL_LINE_DETECT_THR)->GetWindowTextW(shtThresholdString);
	if (!shtThresholdString.IsEmpty())		shtThreshold = (int)wcstod(shtThresholdString, NULL);
}

void CMy081226_camera_param_PFDlg::updateResult() {
	CString s;
	s.Format(L"%6d", (int)timeDiff_captureImg);			this->GetDlgItem(IDC_STATIC_CAPTURE_IMG_TIME)->SetWindowTextW((LPCTSTR)s); 

	// Measure using Entropy of 1st derivative img
	s.Format(L"%02d_%02d", expLevel_max, focusLevel_max);	this->GetDlgItem(IDC_STATIC_MEASURE_ENTROPY_SCAN_ALL_IMG)->SetWindowTextW((LPCTSTR)s); 
	s.Format(L"%5.3f", maxEntropy);							this->GetDlgItem(IDC_STATIC_MEASURE_ENTROPY_SCAN_ALL_ENTROPY)->SetWindowTextW((LPCTSTR)s); 
	s.Format(L"%6d", (int)timeDiff_measureEntropy);			this->GetDlgItem(IDC_STATIC_MEASURE_ENTROPY_TIME)->SetWindowTextW((LPCTSTR)s); 

	// Measure using Energy of LP filtered img grad
	s.Format(L"%02d_%02d", expLevel_max, focusLevel_max);	this->GetDlgItem(IDC_STATIC_ENERGY_LP_RESULT_IMG)->SetWindowTextW((LPCTSTR)s); 
	s.Format(L"%5.3f", maxEnergy);							this->GetDlgItem(IDC_STATIC_ENERGY_LP_RESULT_ENERGY)->SetWindowTextW((LPCTSTR)s); 
	s.Format(L"%6d", (int)timeDiff_measureEnergyLP);		this->GetDlgItem(IDC_STATIC_ENERGY_LP_TIME)->SetWindowTextW((LPCTSTR)s); 

	// PF
	s.Format(L"%02d_%02d", pfResult.x, pfResult.y);		this->GetDlgItem(IDC_STATIC_PF_IMG)->SetWindowTextW((LPCTSTR)s); 	
	s.Format(L"%6d", (int)timeDiff_pf);					this->GetDlgItem(IDC_STATIC_PF_TIME)->SetWindowTextW((LPCTSTR)s); 

	// tools
	s.Format(L"%2d", (int)nDetectedLines);				this->GetDlgItem(IDC_STATIC_N_DETECTED_LINES)->SetWindowTextW((LPCTSTR)s); 	
}

// ------------------

bool CMy081226_camera_param_PFDlg::isCheckExposure() {
	return (((CButton *) this->GetDlgItem(IDC_CHECK_CAPTURE_IMG_IS_EXPOSURE))->GetCheck() == BST_CHECKED ? true : false);
}

bool CMy081226_camera_param_PFDlg::isCheckFocus() {
	return (((CButton *) this->GetDlgItem(IDC_CHECK_CAPTURE_IMG_IS_FOCUS))->GetCheck() == BST_CHECKED ? true : false);
}

bool CMy081226_camera_param_PFDlg::isCheckImgSetPrevious() {
	return (((CButton *) this->GetDlgItem(IDC_CHECK_IMG_SET_PREVIOUS))->GetCheck() == BST_CHECKED ? true : false);
}

void CMy081226_camera_param_PFDlg::captureFrame(IplImage* img) {
	cvReleaseImage(&img);
	img = NULL;
	img = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
	camera.captureFrameToIplImage(img);
}

bool CMy081226_camera_param_PFDlg::saveEntropyArrayToFile(LPCTSTR filename, double* total, int expFrom, int expTo, int focusFrom, int focusTo) {
	CStdioFile f;	f.Open(filename, CFile::modeCreate | CFile::modeWrite);

	CString text;
	int count = 0;
	for (int expLevel = expFrom; expLevel < expTo; expLevel++) {
		for (int focusLevel = focusFrom; focusLevel < focusTo; focusLevel++) {
			text.Format(L"%02d_%02d    %5.3f\n", expLevel, focusLevel, total[count++]);
			f.WriteString(text);
		}
	}

	f.Close();
	return true;
}

