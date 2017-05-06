#ifndef __BLUEEYE
#define __BLUEEYE

#ifdef ___EXPORTS
#define _EXPORT __declspec(dllexport)
#else
#define _EXPORT __declspec(dllimport)
#endif

_EXPORT BOOL BlueEye_Open(int unit);
_EXPORT void BlueEye_Close(int unit);
_EXPORT int BlueEye_Find();
_EXPORT void BlueEye_InitVideo(int unit, int chan, BOOL cardsync, BOOL prog, BOOL dblspd, BOOL trggrab);
_EXPORT void BlueEye_EnableVideo(int unit, int chan, BOOL enable);
_EXPORT void BlueEye_SetSyncPol(int unit, int chan, BOOL hsync, BOOL vsync, BOOL expo);
_EXPORT void BlueEye_SetupEvenOdd(int unit,int chan,BOOL polarity);
_EXPORT void BlueEye_SelectCamera(int unit,int chan,int camera);
_EXPORT void BlueEye_ReadMem(int unit,PVOID buffer,ULONG baseaddr,ULONG offset,ULONG wait,ULONG size,ULONG count);
_EXPORT void BlueEye_WriteMem(int unit,PVOID buffer,ULONG baseaddr,ULONG offset,ULONG wait,ULONG size,ULONG count);
            
_EXPORT ULONG BlueEye_WaitVsync(int unit,int chan,ULONG timeout_msec);
_EXPORT UCHAR *BlueEye_WaitCapture(int unit,int chan,ULONG vsynccnt,BOOL grab,ULONG msec);
_EXPORT BOOL BlueEye_WaitDualCapture(int unit,ULONG vsynccnt,BOOL grab,ULONG msec,UCHAR **frame0,UCHAR **frame1);
_EXPORT UCHAR *BlueEye_GetCaptureFrame(int unit,int chan);
_EXPORT BOOL BlueEye_IsOddField(int unit,int chan);
_EXPORT void BlueEye_SetIntrPol(int unit,ULONG pol);
_EXPORT ULONG BlueEye_WaitInput(int unit,ULONG timeout_msec);
_EXPORT BOOL BlueEye_GetFrameBufferAddr(int unit,int chan,unsigned char *frame[]);
_EXPORT void BlueEye_SetIntrMask(int unit,ULONG mask);
_EXPORT void BlueEye_EnableIoIntr(int unit,BOOL enable);
_EXPORT void BlueEye_Grab(int unit,int chan,BOOL grab);

_EXPORT void BlueEye_SelectLUT(int unit,int chan, int sel);
_EXPORT void BlueEye_LoadLUT(int unit,int chan, int sel,UCHAR *lut);
_EXPORT ULONG BlueEye_ReadInput(int unit);
_EXPORT ULONG BlueEye_ReadOutput(int unit);
_EXPORT void BlueEye_WriteOutput(int unit,ULONG val);

_EXPORT void BlueEye_SetGrabTiming(int unit,int chan,UCHAR *reg);
_EXPORT void BlueEye_SetExposure(int unit,int chan,int shutter);
_EXPORT ULONG BlueEye_WritePWM(int unit, int ch, ULONG data);
_EXPORT ULONG BlueEye_ReadPWM(int unit, int ch);
_EXPORT void  BlueEye_EnablePWM(int unit, int ch, BOOL enable);
_EXPORT void BlueEye_SetTrgControl(int unit, int chan, BOOL blk, BOOL dly, BOOL pol, BOOL trg);
_EXPORT void BlueEye_Trigger(int unit,int chan);
_EXPORT void BlueEye_SetTrgDelay(int unit, int chan, int delay_us, int blk_ms);

_EXPORT ULONG BlueEye_LockMem(int unit,PVOID buffer,ULONG count);
_EXPORT void BlueEye_UnlockMem(int unit,PVOID buffer);
_EXPORT void BlueEye_FlushCache(int unit,PVOID buffer,ULONG count);
//_EXPORT BOOL BlueEye_SetupGrab(int unit,BOOL color,BOOL intr,PVOID buf0,PVOID buf1,ULONG bufsize);

_EXPORT void BlueEye_Delay(int unit,ULONG microsec);
_EXPORT void BlueEye_memcpyMMX(void *d,void *s,unsigned long n);		// 16 bytes aligned
_EXPORT void BlueEye_YUVtoRGB(UCHAR r,UCHAR g,UCHAR b,UCHAR y,UCHAR u,UCHAR v);
_EXPORT void BlueEye_YUVtoRGB24(UCHAR *d,UCHAR *s,ULONG n);
_EXPORT void BlueEye_YUVtoBGR24(UCHAR *d,UCHAR *s,ULONG n);
_EXPORT void BlueEye_YUVtoRGB32(UCHAR *d,UCHAR *s,ULONG n);

_EXPORT BOOL BlueEye_CreateThread(int unit);
_EXPORT void BlueEye_HookDisplayThread(int unit,UCHAR *(*func)(UCHAR *));

_EXPORT void BlueEye_SetDisplayChannel(int unit,int chan);
_EXPORT ULONG BlueEye_GetDisplayCount(int unit);
_EXPORT void BlueEye_FreezeDisplay(int unit,BOOL freeze);	/* grab은 하면서 단지 display만 멈춘다. */
_EXPORT void BlueEye_SetWindow(int unit,HWND hWnd);
_EXPORT void BlueEye_SetScreen(int unit,int screen);
_EXPORT void BlueEye_SetCrop(int unit,BOOL crop);
_EXPORT UCHAR *BlueEye_GetDisplayFrame(int unit);


// display related functions (indepentent of BlueEye functions)
_EXPORT BOOL BlueEye_AreOverlaysSupported();
_EXPORT BOOL BlueEye_DDrawCreateScreen(int screen,COLORREF keycolor);
_EXPORT void BlueEye_DDrawCloseScreen(int screen);
_EXPORT void BlueEye_SetKeyColor(COLORREF keycolor);
_EXPORT void BlueEye_FillMargin(HWND hWnd,HDC hDC);
_EXPORT void BlueEye_FillKeyColor(HWND hWnd,HDC hDC,BOOL Crop);
_EXPORT void BlueEye_MoveOverlay(HWND hWnd,BOOL crop);	/* overlay screen의 위치를 hWnd의 위치로 옮긴다. */
_EXPORT void BlueEye_Draw(HWND hWnd,HDC hdc,int screen,BOOL crop);	/* screen을 hWnd에 도시한다.(overlay의 경우, 위치만 update한다.) */
_EXPORT void BlueEye_UpdateScreen(HWND hWnd,int screen,UCHAR *frame,BOOL color,BOOL crop);	/* frame에 있는 color 상태의 영상을 screen에 copy한 후, hWnd에 도시한다. */


#define OVERLAYSCREEN (-1)
#define FOREVER 0

#define BLUEEYE_WIDTH 640
#define BLUEEYE_HEIGHT 480

#define BLUEEYE_FIELD (BLUEEYE_HEIGHT/2)

#endif
