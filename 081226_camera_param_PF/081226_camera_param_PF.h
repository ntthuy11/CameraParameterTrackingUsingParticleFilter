// 081226_camera_param_PF.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CMy081226_camera_param_PFApp:
// See 081226_camera_param_PF.cpp for the implementation of this class
//

class CMy081226_camera_param_PFApp : public CWinApp
{
public:
	CMy081226_camera_param_PFApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CMy081226_camera_param_PFApp theApp;