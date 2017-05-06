#ifndef __CBLUEEYE
#define __CBLUEEYE

#ifdef ___EXPORTS
#define _EXPORT __declspec(dllexport)
#else
#define _EXPORT __declspec(dllimport)
#endif

#ifndef FOREVER
#define FOREVER 0
#endif

#define BLUEEYE_WIDTH 640
#define BLUEEYE_HEIGHT 480
#define BLUEEYE_FIELD (BLUEEYE_HEIGHT/2)

#define BLUEEYE_SUCCESS 0
#define BLUEEYE_SCREEN_ERR 1
#define BLUEEYE_CARD_ERR 2
#define BLUEEYE_NOT_INIT 3

class CBlueEye {
private:
protected:
public:
	int m_Unit;
	BOOL m_Stat;
	int m_Screen;
	int m_ScreenType;
	HWND m_hWnd;
	BOOL m_Crop;
	BOOL m_Color;
	BOOL m_Capture; 
	int m_DisplayChannel;
	COLORREF m_KeyColor;

	_EXPORT CBlueEye();
	_EXPORT ~CBlueEye();

	_EXPORT BOOL CBlueEye::Initialize(int unit,int screentype=0);
	_EXPORT long CBlueEye::Status(BOOL msgbox);
	_EXPORT int CBlueEye::GetScreenType();
	_EXPORT void CBlueEye::InitVideo(int chan, BOOL cardsync, BOOL prog, BOOL dblspd, BOOL trggrab);
	_EXPORT void CBlueEye::EnableVideo(int chan, BOOL enable);
	_EXPORT void CBlueEye::SetSyncPol(int chan, BOOL hsync, BOOL vsync, BOOL expo);
	_EXPORT void CBlueEye::SetupEvenOdd(int chan,BOOL polarity);
	_EXPORT void CBlueEye::SelectCamera(int chan, int camera);
	_EXPORT UCHAR *CBlueEye::WaitCapture(int chan, ULONG vsynccnt,BOOL backgrab,ULONG timeout_msec);
	_EXPORT BOOL CBlueEye::WaitDualCapture(ULONG vsynccnt,BOOL backgrab,ULONG timeout_msec,UCHAR **frame0,UCHAR **frame1);
	_EXPORT UCHAR *CBlueEye::GetCaptureFrame(int chan);
	_EXPORT BOOL CBlueEye::IsOddField(int chan);
	_EXPORT ULONG CBlueEye::WaitInput(ULONG timeout_msec);
	_EXPORT void CBlueEye::Unfreeze();
	_EXPORT void CBlueEye::SelectLUT(int chan,int sel);
	_EXPORT void CBlueEye::LoadLUT(int chan,int sel,UCHAR *lut);
	_EXPORT void  CBlueEye::EnableIntr(BOOL enable);
	_EXPORT void CBlueEye::SetIntrPol(ULONG pol);
	_EXPORT void CBlueEye::SetIntrMask(ULONG mask);
	_EXPORT ULONG CBlueEye::ReadInput();
	_EXPORT ULONG CBlueEye::ReadOutput();

	_EXPORT void CBlueEye::WriteOutput(ULONG val);
	_EXPORT ULONG CBlueEye::WritePWM(int ch, ULONG data);
	_EXPORT ULONG CBlueEye::ReadPWM(int ch);
	_EXPORT void CBlueEye::EnablePWM(int ch, BOOL enable);
	_EXPORT void CBlueEye::SetExposure(int chan,int shutter);
	_EXPORT void CBlueEye::SetGrabTiming(int chan,UCHAR *reg);
	_EXPORT void CBlueEye::SetTrgControl(int chan, BOOL blk, BOOL dly, BOOL pol, BOOL trg);
	_EXPORT void CBlueEye::Trigger(int unit,int chan);
	_EXPORT void CBlueEye::SetTrgDelay(int chan,int delay_us, int blk_ms);
	_EXPORT void CBlueEye::Delay(ULONG microsec);
	_EXPORT void CBlueEye::HookDisplayThread(UCHAR *(*func)(UCHAR *));
	_EXPORT void CBlueEye::SetWindow(HWND hwnd);
	_EXPORT void CBlueEye::SetDisplayChannel(int chan);
	_EXPORT ULONG CBlueEye::GetDisplayCount();
	_EXPORT void CBlueEye::SetKeyColor(COLORREF color);
	_EXPORT void CBlueEye::MoveScreen();
	_EXPORT void CBlueEye::Draw(HDC hdc);
	_EXPORT void CBlueEye::UpdateScreen(UCHAR *frame,BOOL color=FALSE);
	_EXPORT void CBlueEye::SetCrop(BOOL crop);
	_EXPORT UCHAR *CBlueEye::GetDisplayFrame();
    
};

#endif