#include "StdAfx.h"
#include "Camera.h"

Camera::Camera(void) { 
	
	// ----- init Camera (display)
/*	int m_Unit = 0;			// card selection
	int m_Inter = 1;        // Camera Type - Progressive (0: Interlaced, 1: Progressive)
	int m_DblSpd = 1;		// camera speed - Double
	int m_Overlay = 0;		// Direct Draw Function - Overlay	
	BOOL m_Hsync = FALSE;
	BOOL m_Vsync = FALSE;
	BOOL m_Expo = FALSE;	
	BOOL m_bTrigGrabMode = FALSE;
	BOOL m_Sync = TRUE;		// camera sync
	BOOL m_EvenOdd = FALSE;

	if (!BlueEye.Initialize(m_Unit, 3))	{ // ScreenType = 3
		BlueEye.Status(TRUE);
		exit(0);
	}
	BlueEye.SetDisplayChannel(0);
	BlueEye.InitVideo(0, m_Sync, m_Inter, m_DblSpd, m_bTrigGrabMode);
	BlueEye.InitVideo(1, m_Sync, m_Inter, m_DblSpd, m_bTrigGrabMode);
	BlueEye.EnableVideo(0,TRUE);				// enable vide channel 0
	BlueEye.EnableVideo(1,TRUE);				// enable vide channel 1
	BlueEye.SetSyncPol(0, m_Hsync, m_Vsync, m_Expo);
	BlueEye.SetSyncPol(1, m_Hsync, m_Vsync, m_Expo);
	BlueEye.SetupEvenOdd(0, m_EvenOdd);	// Not necessary in general
	BlueEye.SetupEvenOdd(1, m_EvenOdd);	// Not necessary in general	

	// ----- init Camera (control)
	serialPort.Close(); 
	serialPort.Open(L"COM1", 9600, 8, NOPARITY, ONESTOPBIT);
	//serialPort.VISCAInitialzation(); // dua camera ve vi tri ban dau

	if (!serialPort.VISCACompletionWrite(BACKLIGHT_OFF))					TRACE0("------ Error in command : BACKLIGHT");

	if (!serialPort.VISCACompletionWrite(APERTURE_RESET))					TRACE0("------ Error in command : APERTURE_RESET");
	if (!serialPort.VISCACompletionWrite(EXPOSURE_COMPENSATE_ON))			TRACE0("------ Error in command : EXPOSURE_COMPENSATE_ON");
	if (!serialPort.VISCACompletionWrite(EXPOSURE_COMPENSATE_RESET))		TRACE0("------ Error in command : EXPOSURE_COMPENSATE_RESET");
	//if (!serialPort.VISCACompletionWrite(EXPOSURE_COMPENSATE_DIRECT_0))	TRACE0("------ Error in command : EXPOSURE_COMPENSATE_DIRECT_0");
	if (!serialPort.VISCACompletionWrite(FOCUS_MANUAL))						TRACE0("------ Error in command : FOCUS_MANUAL");
	if (!serialPort.VISCACompletionWrite(FOCUS_NEAR_LIMIT))					TRACE0("------ Error in command : FOCUS_NEAR_LIMIT");

	// ----- init params
	aprSteps[0] = APERTURE_DIRECT_0;
	aprSteps[1] = APERTURE_DIRECT_1;
	aprSteps[2] = APERTURE_DIRECT_2;
	aprSteps[3] = APERTURE_DIRECT_3;
	aprSteps[4] = APERTURE_DIRECT_4;
	aprSteps[5] = APERTURE_DIRECT_5;
	aprSteps[6] = APERTURE_DIRECT_6;
	aprSteps[7] = APERTURE_DIRECT_7;
	aprSteps[8] = APERTURE_DIRECT_8;
	aprSteps[9] = APERTURE_DIRECT_9;
	aprSteps[10] = APERTURE_DIRECT_10;
	aprSteps[11] = APERTURE_DIRECT_11;
	aprSteps[12] = APERTURE_DIRECT_12;
	aprSteps[13] = APERTURE_DIRECT_13;
	aprSteps[14] = APERTURE_DIRECT_14;
	aprSteps[15] = APERTURE_DIRECT_15;

	expSteps[0] = EXPOSURE_COMPENSATE_DIRECT_0;
	expSteps[1] = EXPOSURE_COMPENSATE_DIRECT_1;
	expSteps[2] = EXPOSURE_COMPENSATE_DIRECT_2;
	expSteps[3] = EXPOSURE_COMPENSATE_DIRECT_3;
	expSteps[4] = EXPOSURE_COMPENSATE_DIRECT_4;
	expSteps[5] = EXPOSURE_COMPENSATE_DIRECT_5;
	expSteps[6] = EXPOSURE_COMPENSATE_DIRECT_6;
	expSteps[7] = EXPOSURE_COMPENSATE_DIRECT_7;
	expSteps[8] = EXPOSURE_COMPENSATE_DIRECT_8;
	expSteps[9] = EXPOSURE_COMPENSATE_DIRECT_9;
	expSteps[10] = EXPOSURE_COMPENSATE_DIRECT_10;
	expSteps[11] = EXPOSURE_COMPENSATE_DIRECT_11;
	expSteps[12] = EXPOSURE_COMPENSATE_DIRECT_12;
	expSteps[13] = EXPOSURE_COMPENSATE_DIRECT_13;
	expSteps[14] = EXPOSURE_COMPENSATE_DIRECT_14;
	
	focusSteps[0] = FOCUS_DIRECT_0;
	focusSteps[1] = FOCUS_DIRECT_1;
	focusSteps[2] = FOCUS_DIRECT_2;
	focusSteps[3] = FOCUS_DIRECT_3;
	focusSteps[4] = FOCUS_DIRECT_4;
	focusSteps[5] = FOCUS_DIRECT_5;
	focusSteps[6] = FOCUS_DIRECT_6;
	focusSteps[7] = FOCUS_DIRECT_7;
	focusSteps[8] = FOCUS_DIRECT_8;
	focusSteps[9] = FOCUS_DIRECT_9;
	focusSteps[10] = FOCUS_DIRECT_10;
	focusSteps[11] = FOCUS_DIRECT_11;
	focusSteps[12] = FOCUS_DIRECT_12;
	focusSteps[13] = FOCUS_DIRECT_13;*/
}

Camera::~Camera(void) { }

uchar* Camera::captureFrameToRaw() { 
	uchar* rawFrame = BlueEye.WaitCapture(0, 1, TRUE, FOREVER); // channel = 0, backGrab = TRUE, timeout = FOREVER	
	rawFrame = trimUcharArray(rawFrame, BLUEEYE_WIDTH, BLUEEYE_HEIGHT, GAP_TOP, GAP_BOTTOM, GAP_LEFT, GAP_RIGHT);
	return rawFrame;
}

void Camera::captureFrameToIplImage(IplImage* imageFrame) {
	uchar* rawFrame = captureFrameToRaw();
	uchar* imageFrameData = (uchar *)imageFrame->imageData;
	int h = imageFrame->height, w = imageFrame->width, step = imageFrame->widthStep;
	for(int i = 0; i < h; i++) for(int j = 0; j < w; j++) imageFrameData[i*step + j] = rawFrame[i*w + j];

	delete[] rawFrame;
}

void Camera::changeAperture(int aprLevel) {
	//if (!serialPort.VISCACompletionWrite(aprSteps[aprLevel]))		TRACE0("------ Error in command : APERTURE_DIRECT");
	serialPort.VISCACompletionWrite(aprSteps[aprLevel]);
}

void Camera::changeExposure(int expLevel) {
	//if (!serialPort.VISCACompletionWrite(expSteps[expLevel]))		TRACE0("------ Error in command : EXPOSURE_COMPENSATE_DIRECT");
	serialPort.VISCACompletionWrite(expSteps[expLevel]);
}

void Camera::changeFocus(int focusLevel) {
	//if (!serialPort.VISCACompletionWrite(focusSteps[focusLevel]))		TRACE0("------ Error in command : FOCUS_DIRECT");
	serialPort.VISCACompletionWrite(focusSteps[focusLevel]);
}

void Camera::release() {
	serialPort.Close(); 
}

// ------------- private -------------

UCHAR* Camera::trimUcharArray(uchar* a, int w, int h, int gapTop, int gapDown, int gapLeft, int gapRight) {
	UCHAR* result = new UCHAR[(w - (gapLeft+gapRight)) * (h - (gapTop+gapDown))];
	int heightEnd = h - gapDown, widthEnd = w - gapRight, c = 0;
	for (int i = gapTop; i < heightEnd; i++) for (int j = gapLeft; j < widthEnd; j++) result[c++] = a[i*w + j];
	return result;
}