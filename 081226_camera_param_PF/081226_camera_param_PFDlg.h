// 081226_camera_param_PFDlg.h : header file
//

#pragma once

#include "cv.h"
#include "highgui.h"
#include "Camera.h"
#include "Histogram.h"
#include "Util.h"
#include "PF.h"
#include "StandardHough.h"

// CMy081226_camera_param_PFDlg dialog
class CMy081226_camera_param_PFDlg : public CDialog
{
// Construction
public:
	CMy081226_camera_param_PFDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MY081226_CAMERA_PARAM_PF_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


private:
	void initGUI();
	void initVariables();
	void releaseResoures();
	void getInput();
	void updateResult();

	bool isCheckExposure();
	bool isCheckFocus();
	bool isCheckImgSetPrevious();

	void captureFrame(IplImage* img);
	bool saveEntropyArrayToFile(LPCTSTR filename, double* total, int expFrom, int expTo, int focusFrom, int focusTo);


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonQuit();
	afx_msg void OnBnClickedButtonCaptureImg();
	afx_msg void OnBnClickedButtonMeasureEntropyScanAll();
	afx_msg void OnBnClickedButtonMeasureEntropyLocalMaxima();
	afx_msg void OnBnClickedButtonMeasureEntropyLocalMaxima2();
	afx_msg void OnBnClickedButtonPfRun();
	afx_msg void OnBnClickedButtonEnergyLpScanAll();
	afx_msg void OnBnClickedButtonToolLoadImg();
	afx_msg void OnBnClickedButtonToolEdgeDetect();
	afx_msg void OnBnClickedButtonToolLineDetect();
};
