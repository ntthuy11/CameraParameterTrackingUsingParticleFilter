#pragma once

#include "cblueeye2.h"
#include "cv.h"
#include "highgui.h"
#include "SerialPort.h"

// ---------------------------------------------------

#define GAP_TOP 16
#define GAP_BOTTOM 8
#define GAP_LEFT 36
#define GAP_RIGHT 8

#define FRAME_WIDTH		596
#define FRAME_HEIGHT	456

#define BACKLIGHT_OFF				"81 01 04 33 03 FF"
#define BACKLIGHT_ON				"81 01 04 33 02 FF"

#define AUTOMATIC_EXPOSURE_MODE		"81 01 04 39 00 FF"		// FULL AUTO default

// --- aperture ---
#define APERTURE_RESET				"81 01 04 02 00 FF"

#define APERTURE_DIRECT_0			"81 01 04 42 00 00 00 00 FF"
#define APERTURE_DIRECT_1			"81 01 04 42 00 00 00 01 FF"
#define APERTURE_DIRECT_2			"81 01 04 42 00 00 00 02 FF"
#define APERTURE_DIRECT_3			"81 01 04 42 00 00 00 03 FF"
#define APERTURE_DIRECT_4			"81 01 04 42 00 00 00 04 FF"
#define APERTURE_DIRECT_5			"81 01 04 42 00 00 00 05 FF"	// init value
#define APERTURE_DIRECT_6			"81 01 04 42 00 00 00 06 FF"
#define APERTURE_DIRECT_7			"81 01 04 42 00 00 00 07 FF"
#define APERTURE_DIRECT_8			"81 01 04 42 00 00 00 08 FF"
#define APERTURE_DIRECT_9			"81 01 04 42 00 00 00 09 FF"
#define APERTURE_DIRECT_10			"81 01 04 42 00 00 00 0A FF"
#define APERTURE_DIRECT_11			"81 01 04 42 00 00 00 0B FF"
#define APERTURE_DIRECT_12			"81 01 04 42 00 00 00 0C FF"
#define APERTURE_DIRECT_13			"81 01 04 42 00 00 00 0D FF"
#define APERTURE_DIRECT_14			"81 01 04 42 00 00 00 0E FF"
#define APERTURE_DIRECT_15			"81 01 04 42 00 00 00 0F FF"

// --- exposure ---
#define EXPOSURE_COMPENSATE_ON		"81 01 04 3E 02 FF"
#define EXPOSURE_COMPENSATE_OFF		"81 01 04 3E 03 FF"
#define EXPOSURE_COMPENSATE_RESET	"81 01 04 0E 00 FF"

#define EXPOSURE_COMPENSATE_DIRECT_0		"81 01 04 4E 00 00 00 00 FF"	// -10.5 dB
#define EXPOSURE_COMPENSATE_DIRECT_1		"81 01 04 4E 00 00 00 01 FF"	// -9 dB
#define EXPOSURE_COMPENSATE_DIRECT_2		"81 01 04 4E 00 00 00 02 FF"	// -7.5 dB
#define EXPOSURE_COMPENSATE_DIRECT_3		"81 01 04 4E 00 00 00 03 FF"	// -6 dB
#define EXPOSURE_COMPENSATE_DIRECT_4		"81 01 04 4E 00 00 00 04 FF"	// -4.5 dB
#define EXPOSURE_COMPENSATE_DIRECT_5		"81 01 04 4E 00 00 00 05 FF"	// -3 dB
#define EXPOSURE_COMPENSATE_DIRECT_6		"81 01 04 4E 00 00 00 06 FF"	// -1.5 dB
#define EXPOSURE_COMPENSATE_DIRECT_7		"81 01 04 4E 00 00 00 07 FF"	// 0 dB
#define EXPOSURE_COMPENSATE_DIRECT_8		"81 01 04 4E 00 00 00 08 FF"	// 1.5 dB
#define EXPOSURE_COMPENSATE_DIRECT_9		"81 01 04 4E 00 00 00 09 FF"	// 3 dB
#define EXPOSURE_COMPENSATE_DIRECT_10		"81 01 04 4E 00 00 00 0A FF"	// 4.5 dB
#define EXPOSURE_COMPENSATE_DIRECT_11		"81 01 04 4E 00 00 00 0B FF"	// 6 dB
#define EXPOSURE_COMPENSATE_DIRECT_12		"81 01 04 4E 00 00 00 0C FF"	// 7.5 dB
#define EXPOSURE_COMPENSATE_DIRECT_13		"81 01 04 4E 00 00 00 0D FF"	// 9 dB
#define EXPOSURE_COMPENSATE_DIRECT_14		"81 01 04 4E 00 00 00 0E FF"	// 10.5 dB

// --- focus ---
#define FOCUS_MANUAL				"81 01 04 38 03 FF"
#define FOCUS_NEAR_LIMIT			"81 01 04 28 08 04 00 00 FF"	// set cai nay len near nha^'t thi moi co the thay doi focus len duoc

#define FOCUS_DIRECT_0				"81 01 04 48 01 01 06 01 FF"	// 20m
#define FOCUS_DIRECT_1				"81 01 04 48 01 01 06 0D FF"	// 18m
#define FOCUS_DIRECT_2				"81 01 04 48 01 02 02 0A FF"	// 16m
#define FOCUS_DIRECT_3				"81 01 04 48 01 02 03 0C FF"	// 14m
#define FOCUS_DIRECT_4				"81 01 04 48 01 02 0F 03 FF"	// 12m
#define FOCUS_DIRECT_5				"81 01 04 48 01 03 0C 02 FF"	// 10m
#define FOCUS_DIRECT_6				"81 01 04 48 01 05 01 0E FF"	// 8m
#define FOCUS_DIRECT_7				"81 01 04 48 01 05 03 06 FF"	// 6m
#define FOCUS_DIRECT_8				"81 01 04 48 01 08 04 04 FF"	// 4m
#define FOCUS_DIRECT_9				"81 01 04 48 02 02 06 0F FF"	// 2m
#define FOCUS_DIRECT_10				"81 01 04 48 03 0F 02 0A FF"	// 1.5m
#define FOCUS_DIRECT_11				"81 01 04 48 04 00 0A 0A FF"	// 1m
#define FOCUS_DIRECT_12				"81 01 04 48 06 02 0C 09 FF"	// 0.5m
#define FOCUS_DIRECT_13				"81 01 04 48 08 02 0C 01 FF"	// 0.1m


// ---------------------------------------------------

class Camera
{
public:
	Camera(void);
public:
	~Camera(void);

	UCHAR * captureFrameToRaw();
	void captureFrameToIplImage(IplImage* imageFrame);	
	void changeAperture(int aprLevel);
	void changeExposure(int expLevel);
	void changeFocus(int focusLevel);
	void release();

private:
	CSerialPort serialPort;
	char* aprSteps[16];
	char* expSteps[15];
	char* focusSteps[14];

	CBlueEye BlueEye;

	UCHAR* trimUcharArray(uchar* a, int w, int h, int gapTop, int gapDown, int gapLeft, int gapRight);
};
